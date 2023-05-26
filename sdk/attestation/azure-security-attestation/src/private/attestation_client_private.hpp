// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "attestation_deserializers_private.hpp"
#include "azure/attestation/attestation_client.hpp"
#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include "crypto/inc/crypto.hpp"
#include "jsonhelpers_private.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  template <class T> class EmptyDeserializer {
  public:
    static T Deserialize(Azure::Core::Json::_internal::json const&) { return T{}; }
  };

  template <class T, class TDeserializer = EmptyDeserializer<T>> class AttestationTokenInternal {
  private:
    Models::AttestationToken<T> m_token;

    /**
     * @brief Validate the time elements in a JSON Web token as controlled by the provided
     * validation options.
     *
     * @param validationOptions Options which control how the time validation is performed.
     *
     * @throws std::runtime_error Thrown when the time in the token is invalid (the token has
     * expired or is not yet valid).
     */
    void ValidateTokenTimeElements(AttestationTokenValidationOptions const& validationOptions) const
    {
      // Snapshot "now" to provide a base time for subsequent checks. Note that this code
      // round-trips the time through time_t to round to the nearest second.
      const time_t timeNowSeconds
          = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      const auto timeNow = std::chrono::system_clock::from_time_t(timeNowSeconds);

      if (m_token.ExpiresOn && validationOptions.ValidateExpirationTime)
      {
        if (timeNow > *m_token.ExpiresOn)
        {
          auto expiresOn = static_cast<std::chrono::system_clock::time_point>(*m_token.ExpiresOn);
          auto timeDelta = timeNow - expiresOn;
          if (timeDelta > validationOptions.TimeValidationSlack)
          {
            std::stringstream ss;
            ss << "Attestation token has expired. Token expiration time: "
               << m_token.ExpiresOn->ToString()
               << ". Current time: " << Azure::DateTime(timeNow).ToString();
            throw std::runtime_error(ss.str());
          }
        }
      }
      if (m_token.NotBefore && validationOptions.ValidateNotBeforeTime)
      {
        if (timeNow < *m_token.NotBefore)
        {
          auto notBefore = static_cast<std::chrono::system_clock::time_point>(*m_token.NotBefore);
          auto timeDelta = notBefore - timeNow;
          if (timeDelta > validationOptions.TimeValidationSlack)
          {
            std::stringstream ss;
            ss << "Attestation token is not yet valid. Token becomes valid at time: "
               << m_token.NotBefore->ToString()
               << ". Current time: " << Azure::DateTime(timeNow).ToString();
            throw std::runtime_error(ss.str());
          }
        }
      }
    }

    /**
     * @brief Validate the issuer of the attestation token based on the validation options provided.
     *
     * @param validationOptions Options controlling the validation
     */
    void ValidateTokenIssuer(AttestationTokenValidationOptions const& validationOptions) const
    {
      if (validationOptions.ValidateIssuer)
      {
        if (!m_token.Issuer)
        {
          throw std::runtime_error(
              "Attestation token issuer validation requested but token has no issuer.");
        }
        if (validationOptions.ExpectedIssuer != *m_token.Issuer)
        {
          std::stringstream ss;
          ss << "Expected issuer (" << validationOptions.ExpectedIssuer
             << ") does not match actual issuer of token (" << *m_token.Issuer << ")";
          throw std::runtime_error(ss.str());
        }
      }
    }

    /**
     * @brief  Find the set of possible signers for this attestation token.
     *
     * @details  If the caller provided a set of signers, then use that set of signers exclusively
     to find
        * the possible signer for this token.

        * Otherwise, inspect the token itself for evidence of the signers - the token header may
        * contain possible signers for this token.

     * @param signers A set of possible signers for this token.
     *
     * @return std::vector<Models::AttestationSigner> The set of possible signers found, or an empty
     array if none were found.
     */
    std::vector<Models::AttestationSigner> FindPossibleSigners(
        std::vector<Models::AttestationSigner> const& signers) const
    {
      std::vector<Models::AttestationSigner> returnValue;
      // If signers is provided, then the Signers array provides the complete set of
      // possible signers for the token.
      if (!signers.empty())
      {
        // If the token header has a Key ID, search the signers for that key ID.
        if (m_token.Header.KeyId)
        {
          for (const auto& signer : signers)
          {
            if (signer.KeyId && *m_token.Header.KeyId == *signer.KeyId)
            {
              returnValue.push_back(signer);
            }
          }
        }
        else
        {
          // We don't have a KeyID in the token, our only option is to return all the potential
          // signers and let the caller sort it out.
          for (const auto& signer : signers)
          {
            returnValue.push_back(signer);
          }
        }
      }
      else
      {
        if (m_token.Header.Key)
        {
          returnValue.push_back(*m_token.Header.Key);
        }
        if (m_token.Header.X509CertificateChain)
        {
          std::vector<std::string> pemEncodedChain;
          for (auto X5c : *m_token.Header.X509CertificateChain)
          {
            pemEncodedChain.push_back(_detail::Cryptography::PemFromBase64(X5c, "CERTIFICATE"));
          }
          returnValue.push_back(
              Models::AttestationSigner{Azure::Nullable<std::string>{}, pemEncodedChain});
        }
      }
      return returnValue;
    }

    /** @brief Given a set of possible signers, verify that the token was signed by one of those
     * signers.
     * @param possibleSigners - a list of possible AttestationSigner objects that could have
     * signed the token.
     * @returns Azure::Nullable<AttestationSigner> - If the return value has a value, it will
     * be the certificate which signed the token, otherwise the token signature could not be
     * validated.
     */
    Azure::Nullable<Models::AttestationSigner> VerifyTokenSignature(
        std::vector<Models::AttestationSigner> const& possibleSigners) const
    {
      for (const auto& signer : possibleSigners)
      {
        const std::unique_ptr<Azure::Security::Attestation::_detail::Cryptography::X509Certificate>
            certificate(Azure::Security::Attestation::_detail::Cryptography::ImportX509Certificate(
                (*signer.CertificateChain)[0]));
        auto publicKey = certificate->GetPublicKey();
        // If the key associated with this certificate signed the token,
        if (publicKey->VerifySignature(
                std::vector<uint8_t>(m_token.SignedElements.begin(), m_token.SignedElements.end()),
                m_token.Signature))
        {
          return signer;
        }
      }
      return Azure::Nullable<Models::AttestationSigner>{};
    }

    // Set the token body based on the bodyToSet parameter provided.
    template <typename Ty>
    void SetTokenBody(Azure::Core::Json::_internal::json const& jsonBody, Ty const* const bodyToSet)
    {
      if (bodyToSet != nullptr)
      {
        m_token.Body = *bodyToSet;
      }
      else
      {
        m_token.Body = TDeserializer::Deserialize(jsonBody);
      }
    }

    // Null token body overload - used when the AttestationTokenInternal has no body.
    void SetTokenBody(Azure::Core::Json::_internal::json const&, void const* const) {}

  public:
    /** @brief Constructs a new instance of an AttestationToken object from a JSON Web Token or JSON
     * Web Signature.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519'>RFC 7519 (JWT)</a> or
     * <a href='https://datatracker.ietf.org/doc/html/rfc7515'>RFC 7517 (JWS)</a> for more
     * information about JWS and JWT objects.
     *
     * @param jwt - the JSON Web Token/JSON Web Signature to be parsed.
     * @param preferredBody - the body to be used instead of the body contained inside the jwt. This
     * allows creating an AttestationTokenInternal with a body whose type does not match the value
     * within the JWT.
     */
    AttestationTokenInternal(std::string const& jwt, T const* const preferredBody = nullptr)
    {
      m_token.RawToken = jwt;

      // Parse the JWT/JWS object. A JWS object consists of 3 Base64Url encoded components
      // separated by '.' characters.

      std::string token(m_token.RawToken);

      // Parse the first base64url encoded element (the JWS header):
      {
        size_t headerIndex = token.find('.');
        if (headerIndex == std::string::npos)
        {
          throw Azure::Core::RequestFailedException("Could not find required . in token.");
        }

        const std::string header(token.substr(0, headerIndex));

        m_token.SignedElements = header;

        const auto jsonHeader(Azure::Core::Json::_internal::json::parse(
            Azure::Core::_internal::Base64Url::Base64UrlDecode(header)));

        m_token.Header
            = Azure::Security::Attestation::_detail::AttestationTokenHeaderSerializer::Deserialize(
                jsonHeader);

        // Remove the header from the token, we've remembered its contents.
        token.erase(0, headerIndex + 1);
      }

      // Append the separator between the header and body elements.
      m_token.SignedElements += '.';

      // Parse the second base64url encoded element (the JWS body):
      {
        const size_t bodyIndex = token.find('.');
        if (bodyIndex == std::string::npos)
        {
          throw Azure::Core::RequestFailedException("Could not find required second . in token.");
        }

        const std::string body(token.substr(0, bodyIndex));

        // Now add the encoded body to the signed elements.
        m_token.SignedElements += body;
        if (!body.empty())
        {
          auto jsonBody(Azure::Core::Json::_internal::json::parse(
              Azure::Core::_internal::Base64Url::Base64UrlDecode(body)));

          // Parse the RFC 7519 JSON Web Token body properties.
          // Note that if this is a JWS, these properties will NOT be present.
          Azure::Core::Json::_internal::JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
              m_token.ExpiresOn,
              jsonBody,
              "exp",
              Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
          Azure::Core::Json::_internal::JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
              m_token.IssuedOn,
              jsonBody,
              "iat",
              Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
          Azure::Core::Json::_internal::JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
              m_token.NotBefore,
              jsonBody,
              "nbf",
              Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
          Azure::Core::Json::_internal::JsonOptional::SetIfExists(m_token.Issuer, jsonBody, "iss");
          Azure::Core::Json::_internal::JsonOptional::SetIfExists(m_token.Subject, jsonBody, "sub");
          Azure::Core::Json::_internal::JsonOptional::SetIfExists(
              m_token.Audience, jsonBody, "aud");
          Azure::Core::Json::_internal::JsonOptional::SetIfExists(
              m_token.UniqueIdentifier, jsonBody, "jti");

          SetTokenBody(jsonBody, preferredBody);
        }
        // Remove the body from the token, we've remembered its contents.
        token.erase(0, bodyIndex + 1);
      }
      // Parse the signature from the remainder of the token.
      {
        std::string signature(token);
        m_token.Signature = Azure::Core::_internal::Base64Url::Base64UrlDecode(signature);
      }
    }

    /**
     * @brief Create a new attestation token object with a body containing the tokenBody provided.
     *
     * @param tokenBody A structure representing the body of the attestation token. The body will be
     * serialized using the {@link TDeserializer} class.
     * @param tokenSigner An optional signer for the token. If provided, will be used to sign the
     * token.
     * @return AttestationTokenInternal<T, TDeserializer> A newly created token object.
     */
    static AttestationTokenInternal<T, TDeserializer> CreateToken(
        Azure::Nullable<T> const& tokenBody,
        Azure::Nullable<AttestationSigningKey> const& tokenSigner = {})
    {
      bool isUnsecuredToken = false;
      std::unique_ptr<Azure::Security::Attestation::_detail::Cryptography::X509Certificate>
          signingCert;
      std::unique_ptr<Azure::Security::Attestation::_detail::Cryptography::AsymmetricKey>
          signingKey;
      Models::AttestationTokenHeader tokenHeader;

      if (tokenSigner)
      {
        // Deserialize the signing key and certificate and use them to create the JWS header.
        signingCert = Azure::Security::Attestation::_detail::Cryptography::ImportX509Certificate(
            tokenSigner->PemEncodedX509Certificate);
        signingKey = Azure::Security::Attestation::_detail::Cryptography::ImportPrivateKey(
            tokenSigner->PemEncodedPrivateKey);

        tokenHeader.Algorithm = signingCert->GetAlgorithm();
        tokenHeader.X509CertificateChain = std::vector<std::string>{signingCert->ExportAsBase64()};
      }
      else
      {
        // If the private key and certificate are empty, it's an unsecured JWS.
        // An unsecured JWS is represented by an "alg" header with a value of "none" and an
        // empty signature block.
        isUnsecuredToken = true;
        tokenHeader.Algorithm = "none"; // Specifies an unsecured attestation token.
      }
      std::string serializedHeader(AttestationTokenHeaderSerializer::Serialize(tokenHeader));
      std::string serializedBody;
      if (tokenBody)
      {
        serializedBody = TDeserializer::Serialize(*tokenBody);
      }

      const std::string encodedHeader = Azure::Core::_internal::Base64Url::Base64UrlEncode(
          std::vector<uint8_t>(serializedHeader.begin(), serializedHeader.end()));
      const std::string encodedBody = Azure::Core::_internal::Base64Url::Base64UrlEncode(
          std::vector<uint8_t>(serializedBody.begin(), serializedBody.end()));

      // Start to assemble the JWT from the encoded header and body.
      std::string jwt = encodedHeader + "." + encodedBody;

      if (isUnsecuredToken)
      {
        // An unsecured token has an empty signature, so at this point, we're done. All we need
        // to do is to append the "." indicating an unsecured JWT.
        jwt += ".";
      }
      else
      {
        // Sign the first two pieces of the JWS
        auto signedBuffer = signingKey->SignBuffer(std::vector<uint8_t>(jwt.begin(), jwt.end()));
        // Append the separator between the signed data (first two components of the JWS) and
        // the signature.
        jwt += ".";
        jwt += Azure::Core::_internal::Base64Url::Base64UrlEncode(signedBuffer);
      }
      return AttestationTokenInternal(jwt);
    }

    /**
     * @brief Validate this attestation token.
     *
     * @note If the "signers" parameter is provided, then only the signers from the
     * "signers" value will be considered when validating the token signature. If no
     * "signers" are provided, then the ValidateToken API will attempt to find signers
     * in the token itself
     *
     * @param validationOptions Options which can be used when validating the token.
     * @param signers Potential signers for this attestation token.
     */
    void ValidateToken(
        AttestationTokenValidationOptions const& validationOptions,
        std::vector<Models::AttestationSigner> const& signers
        = std::vector<Models::AttestationSigner>{}) const
    {
      if (!validationOptions.ValidateToken)
      {
        return;
      }

      // If this is a secured token, find a set of possible signers for the token and
      // verify that one of them signed the token.
      Azure::Nullable<Models::AttestationSigner> tokenSigner;
      if (m_token.Header.Algorithm && *m_token.Header.Algorithm != "none"
          && validationOptions.ValidateSigner)
      {
        tokenSigner = VerifyTokenSignature(FindPossibleSigners(signers));
        if (!tokenSigner)
        {
          throw std::runtime_error("Unable to verify the attestation token signature.");
        }
      }

      // Now check the expiration time
      ValidateTokenTimeElements(validationOptions);

      // And finally check the issuer.
      ValidateTokenIssuer(validationOptions);

      if (validationOptions.ValidationCallback)
      {
        AttestationTokenInternal<void> tokenForCallback(m_token.RawToken);
        validationOptions.ValidationCallback(
            tokenForCallback, tokenSigner ? *tokenSigner : Models::AttestationSigner());
      }
    }

    /**
     * @brief Convert the internal attestation token to a public AttestationToken object.
     */
    operator Models::AttestationToken<T>&() { return m_token; }
    /**
     * @brief Convert the internal attestation token to a public AttestationToken object.
     */
    operator Models::AttestationToken<T> const &() const { return m_token; }
  };
}}}} // namespace Azure::Security::Attestation::_detail

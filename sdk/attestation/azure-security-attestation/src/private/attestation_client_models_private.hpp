// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include "azure/core/base64.hpp"
#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/core/response.hpp"
#include "crypto/inc/crypto.hpp"
#include "jsonhelpers.hpp"
#include "jsonwebkeyset.hpp"
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  /**
   * @brief Internal implementation class implementing private methods for public model type
   * AttestationSigner.
   */
  class AttestationSignerInternal {
    AttestationSigner m_signer;

  public:
    AttestationSignerInternal(Azure::Security::Attestation::_detail::JsonWebKey const& jwk)
    {
      m_signer.KeyId = jwk.kid;
      if (jwk.x5c.HasValue())
      {
        m_signer.CertificateChain = std::vector<std::string>();
        for (const auto& x5c : jwk.x5c.Value())
        {
          m_signer.CertificateChain.Value().push_back(PemFromX5c(x5c));
        }
      }
    }

    static std::string PemFromX5c(std::string const& x5c)
    {
      std::string rv;
      rv += "-----BEGIN CERTIFICATE-----\r\n";
      std::string encodedKey(x5c);

      // Insert crlf characters every 80 characters into the base64 encoded key to make it
      // prettier.
      size_t insertPos = 80;
      while (insertPos < encodedKey.length())
      {
        encodedKey.insert(insertPos, "\r\n");
        insertPos += 82; /* 80 characters plus the \r\n we just inserted */
      }

      rv += encodedKey;
      rv += "\r\n-----END CERTIFICATE-----\r\n";
      return rv;
    }
    operator AttestationSigner&&() { return std::move(m_signer); }
    static Azure::Core::Json::_internal::json SerializeToJson(AttestationSigner const& signer)
    {
      Azure::Core::Json::_internal::json rv;

      if (signer.KeyId.HasValue())
      {
        rv["kid"] = signer.KeyId.Value();
      }
      if (signer.CertificateChain.HasValue())
      {
        rv["x5c"] = signer.CertificateChain.Value();
      }
      return rv;
    }
  };

  struct AttestationTokenHeaderSerializer final
  {
    static AttestationTokenHeader Deserialize(Azure::Core::Json::_internal::json const& jsonHeader)
    {
      AttestationTokenHeader returnValue;

      returnValue.Algorithm
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "alg");
      returnValue.KeyURL
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "jku");

      if (jsonHeader.contains("jwk"))
      {
        Azure::Security::Attestation::_detail::JsonWebKey jwk
            = Azure::Security::Attestation::_detail::JsonWebKeySerializer::Deserialize(
                jsonHeader["jwk"]);
        returnValue.Key
            = Azure::Security::Attestation::Models::_detail::AttestationSignerInternal(jwk);
      }
      returnValue.KeyId
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "kid");
      returnValue.X509Url
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "x5u");
      returnValue.X509CertificateChain
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
              jsonHeader, "x5c");
      returnValue.CertificateSha256Thumbprint
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(
              jsonHeader, "x5t#S256");
      returnValue.CertificateThumbprint
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "x5t");
      returnValue.Type
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "typ");
      returnValue.ContentType
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonHeader, "cty");
      returnValue.Critical
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
              jsonHeader, "crit");
      return returnValue;
    }
    static std::string Serialize(AttestationTokenHeader const& tokenHeader)
    {
      Azure::Core::Json::_internal::json serializedHeader;
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.Algorithm, "alg");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.KeyURL, "jku");
      if (tokenHeader.Key.HasValue())
      {
        auto jsonSigner(AttestationSignerInternal::SerializeToJson(tokenHeader.Key.Value()));
        Azure::Security::Attestation::_detail::JsonHelpers::SetField(
            serializedHeader, jsonSigner, "jwk");
      }
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.ContentType, "cty");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.Critical, "crit");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.KeyId, "kid");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.Type, "typ");

      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.X509CertificateChain, "x5c");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.X509Url, "x5u");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.CertificateSha256Thumbprint, "x5t#S256");
      Azure::Security::Attestation::_detail::JsonHelpers::SetField(
          serializedHeader, tokenHeader.CertificateThumbprint, "x5t");

      return serializedHeader.dump();
    }
  };

  template <class T, typename TDeserializer> class AttestationTokenInternal {
  private:
    AttestationToken<T> m_token;

  public:
    /** Constructs a new instance of an AttestationToken object from a JSON Web Token or JSON Web
     * Signature.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519'>RFC 7519 (JWT)</a> or
     * <a href='https://datatracker.ietf.org/doc/html/rfc7515'>RFC 7517 (JWS)</a> for more
     * information about JWS and JWT objects.
     *
     * @param jwt - the JSON Web Token/JSON Web Signature to be parsed.
     */
    AttestationTokenInternal(std::string const& jwt)
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
        std::string header(token);
        header.erase(headerIndex);

        m_token.SignedElements = header;

        auto jsonHeader(Azure::Core::Json::_internal::json::parse(
            Azure::Core::_internal::Base64Url::Base64UrlDecode(header)));

        m_token.Header = AttestationTokenHeaderSerializer::Deserialize(jsonHeader);

        // Remove the header from the token, we've remembered its contents.
        token.erase(0, headerIndex + 1);
      }

      // Append the separator between the header and body elements.
      m_token.SignedElements += '.';

      // Parse the second base64url encoded element (the JWS body):
      {
        size_t bodyIndex = token.find('.');
        if (bodyIndex == std::string::npos)
        {
          throw Azure::Core::RequestFailedException("Could not find required second . in token.");
        }

        std::string body(token);
        body.erase(bodyIndex);

        // Now add the encoded body to the signed elements.
        m_token.SignedElements += body;

        auto jsonBody(Azure::Core::Json::_internal::json::parse(
            Azure::Core::_internal::Base64Url::Base64UrlDecode(body)));

        // Parse the RFC 7519 JSON Web Token body properties.
        // Note that if this is a JWS, these properties will NOT be present.
        m_token.ExpiresOn = Azure::Security::Attestation::_detail::JsonHelpers::ParseDateTimeField(
            jsonBody, "exp");
        m_token.IssuedOn = Azure::Security::Attestation::_detail::JsonHelpers::ParseDateTimeField(
            jsonBody, "iat");
        m_token.NotBefore = Azure::Security::Attestation::_detail::JsonHelpers::ParseDateTimeField(
            jsonBody, "nbf");
        m_token.Issuer
            = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonBody, "iss");
        m_token.Subject
            = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonBody, "sub");
        m_token.Audience
            = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonBody, "aud");
        m_token.UniqueIdentifier
            = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jsonBody, "jti");

        m_token.Body = TDeserializer::Deserialize(jsonBody);

        // Remove the body from the token, we've remembered its contents.
        token.erase(0, bodyIndex + 1);
      }
      // Parse the signature from the remainder of the token.
      {
        std::string signature(token);
        m_token.Signature = Azure::Core::_internal::Base64Url::Base64UrlDecode(signature);
      }
    }

    // Returns a signed Attestation Token created with the specified body.
    static AttestationTokenInternal<T, TDeserializer> CreateToken(
        T const& tokenBody,
        AttestationSigningKey const& tokenSigner = AttestationSigningKey{})
    {
      bool isUnsecuredToken = false;
      std::unique_ptr<Azure::Security::Attestation::_private::Cryptography::X509Certificate>
          signingCert;
      std::unique_ptr<Azure::Security::Attestation::_private::Cryptography::AsymmetricKey>
          signingKey;
      AttestationTokenHeader tokenHeader;
      if (tokenSigner.PemEncodedPrivateKey.empty() && tokenSigner.PemEncodedX509Certificate.empty())
      {
        // If the private key and certificate are empty, it's an unsecured JWS.
        // An unsecured JWS is represented by an "alg" header with a value of "none" and an
        // empty signature block.
        isUnsecuredToken = true; // This is an unsecured token, don't sign it.
        tokenHeader.Algorithm = "none"; // Specifies an unsecured attestation token.
      }
      else
      {
        // Deserialize the signing key and certificate and use them to create the JWS header.
        signingCert
            = Azure::Security::Attestation::_private::Cryptography::Crypto::ImportX509Certificate(
                tokenSigner.PemEncodedX509Certificate);
        signingKey = Azure::Security::Attestation::_private::Cryptography::Crypto::ImportPrivateKey(
            tokenSigner.PemEncodedPrivateKey);

        tokenHeader.Algorithm = signingCert->GetAlgorithm();
        tokenHeader.Type = signingCert->GetKeyType();
        tokenHeader.X509CertificateChain = std::vector<std::string>{signingCert->ExportAsBase64()};
      }
      std::string serializedHeader(AttestationTokenHeaderSerializer::Serialize(tokenHeader));
      std::string serializedBody(TDeserializer::Serialize(tokenBody));
      std::string encodedHeader = Azure::Core::_internal::Base64Url::Base64UrlEncode(
          std::vector<uint8_t>(serializedHeader.begin(), serializedHeader.end()));
      std::string encodedBody = Azure::Core::_internal::Base64Url::Base64UrlEncode(
          std::vector<uint8_t>(serializedBody.begin(), serializedBody.end()));

      // Start to assemble the JWT from the encoded header and body.
      std::string jwt = encodedHeader + "." + encodedBody;

      if (isUnsecuredToken)
      {
        // An unsecured token has an empty signature, so at this point, we're done. All we need to
        // do is to append the "." indicating an unsecured JWT.
        jwt += ".";
      }
      else
      {
        // Sign the first two pieces of the JWS
        auto signedBuffer = signingKey->SignBuffer(std::vector<uint8_t>(jwt.begin(), jwt.end()));
        // Append the separator between the signed data (first two components of the JWS) and the
        // signature.
        jwt += ".";
        jwt += Azure::Core::_internal::Base64Url::Base64UrlEncode(signedBuffer);
      }
      return AttestationTokenInternal(jwt);
    }

    std::vector<AttestationSigner> FindPossibleSigners(
        std::vector<AttestationSigner> const& signers)
    {
      std::vector<AttestationSigner> returnValue;
      // If signers is provided, then the Signers array provides the complete set of
      // possible signers for the token.
      if (!signers.empty())
      {
        // If the token header has a Key ID, search the signers for that key ID.
        if (m_token.Header.KeyId.HasValue())
        {
          for (const auto& signer : signers)
          {
            if (signer.KeyId.HasValue() && m_token.Header.KeyId.Value() == signer.KeyId.Value())
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
        if (m_token.Header.Key.HasValue())
        {
          returnValue.push_back(m_token.Header.Key.Value());
        }
        if (m_token.Header.X509CertificateChain.HasValue())
        {
          std::vector<std::string> pemEncodedChain;
          for (auto x5c : m_token.Header.X509CertificateChain.Value())
          {
            pemEncodedChain.push_back(AttestationSignerInternal::PemFromX5c(x5c));
          }
          returnValue.push_back(AttestationSigner{Azure::Nullable<std::string>(), pemEncodedChain});
        }
      }
      return returnValue;
    }

    /// @brief Given a set of possible signers, verify that the token was signed by one of those
    /// signers.
    /// @param possibleSigners - a list of possible AttestationSigner objects that could have signed
    /// the token.
    /// @returns Azure::Nullable<AttestationSigner> - If the return value has a value, it will be
    /// the certificate which signed the token, otherwise the token signature could not be
    /// validated.
    Azure::Nullable<AttestationSigner> VerifyTokenSignature(
        std::vector<AttestationSigner> const& possibleSigners)
    {
      for (const auto& signer : possibleSigners)
      {
        std::unique_ptr<Azure::Security::Attestation::_private::Cryptography::X509Certificate>
            certificate(
                Azure::Security::Attestation::_private::Cryptography::Crypto::ImportX509Certificate(
                    signer.CertificateChain.Value()[0]));
        auto publicKey = certificate->GetPublicKey();
        // If the key associated with this certificate signed the token,
        if (publicKey->VerifySignature(
                std::vector<uint8_t>(m_token.SignedElements.begin(), m_token.SignedElements.end()),
                m_token.Signature))
        {
          return signer;
        }
      }
      return Azure::Nullable<AttestationSigner>();
    }

    void ValidateTokenTimeElements(AttestationTokenValidationOptions const& validationOptions)
    {
      // Snapshot "now" to provide a base time for subsequent checks. Note that this code
      // round-trips the time through time_t to round to the nearest second.
      time_t timeNowSeconds
          = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      auto timeNow = std::chrono::system_clock::from_time_t(timeNowSeconds);

      if (m_token.ExpiresOn.HasValue() && validationOptions.ValidateExpirationTime)
      {
        if (timeNow > m_token.ExpiresOn.Value())
        {
          auto expiresOn
              = static_cast<std::chrono::system_clock::time_point>(m_token.ExpiresOn.Value());
          auto timeDelta = timeNow - expiresOn;
          if (timeDelta > validationOptions.ValidationTimeSlack)
          {
            std::stringstream ss;
            ss << "Attestation token has expired. Token expiration time: "
               << m_token.ExpiresOn.Value().ToString()
               << ". Current time: " << Azure::DateTime(timeNow).ToString();
            throw std::runtime_error(ss.str());
          }
        }
      }
      if (m_token.NotBefore.HasValue() && validationOptions.ValidateNotBeforeTime)
      {
        if (timeNow < m_token.NotBefore.Value())
        {
          auto notBefore
              = static_cast<std::chrono::system_clock::time_point>(m_token.NotBefore.Value());
          auto timeDelta = notBefore - timeNow;
          if (timeDelta > validationOptions.ValidationTimeSlack)
          {
            std::stringstream ss;
            ss << "Attestation token is not yet valid. Token becomes valid at time: "
               << m_token.NotBefore.Value().ToString()
               << ". Current time: " << Azure::DateTime(timeNow).ToString();
            throw std::runtime_error(ss.str());
          }
        }
      }
    }

    /// @brief: Validate this attestation token.
    /// @param validationOptions - Options which can be used when validating the token.
    /// @param signers - Potential signers for this attestation token.
    void ValidateToken(
        AttestationTokenValidationOptions const& validationOptions,
        std::vector<AttestationSigner> const& signers = std::vector<AttestationSigner>{})
    {
      if (!validationOptions.ValidateToken)
      {
        return;
      }

      // If this is a secured token, find a set of possible signers for the token and
      // verify that one of them signed the token.
      if (m_token.Header.Algorithm.HasValue() && m_token.Header.Algorithm.Value() != "none"
          && validationOptions.ValidateSigner)
      {
        Azure::Nullable<AttestationSigner> foundSigner
            = VerifyTokenSignature(FindPossibleSigners(signers));
        if (!foundSigner.HasValue())
        {
          throw std::runtime_error("Unable to verify the attestation token signature.");
        }
      }

      // Now check the expiration time
      ValidateTokenTimeElements(validationOptions);
    }

    operator AttestationToken<T>&&() { return std::move(m_token); }
  };

  struct AttestSgxEnclaveRequest
  {
    std::vector<uint8_t> Quote;
    Azure::Nullable<AttestationData> InitTimeData;
    Azure::Nullable<AttestationData> RunTimeData;
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    Azure::Nullable<std::string> Nonce;
  };
  struct AttestOpenEnclaveRequest
  {
    std::vector<uint8_t> Report;
    Azure::Nullable<AttestationData> InitTimeData;
    Azure::Nullable<AttestationData> RunTimeData;
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    Azure::Nullable<std::string> Nonce;
  };

}}}}} // namespace Azure::Security::Attestation::Models::_detail

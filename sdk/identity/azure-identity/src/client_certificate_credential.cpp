// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_certificate_credential.hpp"

#include "private/token_credential_impl.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/uuid.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

using namespace Azure::Identity;

ClientCertificateCredential::ClientCertificateCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_tokenCredentialImpl(new _detail::TokenCredentialImpl(options)), m_pkey(nullptr)
{
  BIO* bio = nullptr;
  X509* x509 = nullptr;
  try
  {
    {
      using Azure::Core::Credentials::AuthenticationException;

      // Open certificate file, then get private key and X509:
      if ((bio = BIO_new_file(clientCertificatePath.c_str(), "r")) != nullptr)
      {
        if ((m_pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr)) == nullptr)
        {
          throw AuthenticationException("Failed to read certificate private key.");
        }

        if ((x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr)) == nullptr)
        {
          static_cast<void>(BIO_seek(bio, 0));
          if ((x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr)) == nullptr)
          {
            throw AuthenticationException("Failed to read certificate private key.");
          }
        }

        BIO_free(bio);
        bio = nullptr;

        // Get certificate thumbprint:
        {
          using Azure::Core::_internal::Base64Url;

          std::string thumbprintHexStr;
          std::string thumbprintBase64Str;
          {
            std::vector<unsigned char> mdVec(EVP_MAX_MD_SIZE);
            {
              unsigned int mdLen = 0;
              const auto digestResult = X509_digest(x509, EVP_sha1(), mdVec.data(), &mdLen);

              X509_free(x509);
              x509 = nullptr;

              if (!digestResult)
              {
                throw AuthenticationException("Failed to get certificate thumbprint.");
              }

              // Drop unused buffer space:
              const auto mdLenSz = static_cast<decltype(mdVec)::size_type>(mdLen);
              if (mdVec.size() > mdLenSz)
              {
                mdVec.resize(mdLenSz);
              }

              // Get thumbprint as hex string:
              {
                std::ostringstream thumbprintStream;
                for (const auto md : mdVec)
                {
                  thumbprintStream << std::uppercase << std::hex << std::setfill('0')
                                   << std::setw(2) << static_cast<int>(md);
                }
                thumbprintHexStr = thumbprintStream.str();
              }
            }

            // Get thumbprint as Base64:
            {
              static_assert(
                  sizeof(decltype(mdVec)::value_type) == sizeof(uint8_t),
                  "mdVec value should be representable as uint8_t");
              thumbprintBase64Str = Base64Url::Base64UrlEncode(
                  reinterpret_cast<const std::vector<uint8_t>&>(mdVec));
            }
          }

          // Form a JWT token:
          const auto tokenHeader = std::string("{\"x5t\":\"") + thumbprintBase64Str
              + "\",\"kid\":\"" + thumbprintHexStr + "\",\"alg\":\"RS256\",\"typ\":\"JWT\"}";

          static_assert(
              sizeof(std::string::value_type) == sizeof(uint8_t),
              "std::string::value_type value should be representable as uint8_t");

          const auto tokenHeaderVec
              = std::vector<std::string::value_type>(tokenHeader.begin(), tokenHeader.end());

          m_tokenHeaderEncoded = Base64Url::Base64UrlEncode(
              reinterpret_cast<const std::vector<uint8_t>&>(tokenHeaderVec));
        }
      }
      else
      {
        throw AuthenticationException("Failed to open certificate file.");
      }
    }

    using Azure::Core::Url;
    {

      m_requestUrl = Url("https://login.microsoftonline.com/");
      m_requestUrl.AppendPath(tenantId);
      m_requestUrl.AppendPath("oauth2/v2.0/token");
    }

    m_tokenPayloadStaticPart = std::string("{\"aud\":\"") + m_requestUrl.GetAbsoluteUrl()
        + "\",\"iss\":\"" + clientId + "\",\"sub\":\"" + clientId + "\",\"jti\":\"";

    {
      std::ostringstream body;
      body << "grant_type=client_credentials"
              "&client_assertion_type="
              "urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer" // cspell:disable-line
              "&client_id="
           << Url::Encode(clientId);

      m_requestBody = body.str();
    }
  }
  catch (...)
  {
    if (bio != nullptr)
    {
      BIO_free(bio);
    }

    if (x509 != nullptr)
    {
      X509_free(x509);
    }

    if (m_pkey != nullptr)
    {
      EVP_PKEY_free(static_cast<EVP_PKEY*>(m_pkey));
    }

    throw;
  }
}

ClientCertificateCredential::ClientCertificateCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    ClientCertificateCredentialOptions const& options)
    : ClientCertificateCredential(
        tenantId,
        clientId,
        clientCertificatePath,
        static_cast<Azure::Core::Credentials::TokenCredentialOptions const&>(options))
{
}

ClientCertificateCredential::~ClientCertificateCredential()
{
  EVP_PKEY_free(static_cast<EVP_PKEY*>(m_pkey));
}

Azure::Core::Credentials::AccessToken ClientCertificateCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_tokenCredentialImpl->GetToken(context, [&]() {
    using _detail::TokenCredentialImpl;
    using Azure::Core::Http::HttpMethod;

    std::ostringstream body;
    body << m_requestBody;
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        body << "&scope=" << TokenCredentialImpl::FormatScopes(scopes, false);
      }
    }

    std::string assertion = m_tokenHeaderEncoded;
    {
      using Azure::Core::_internal::Base64Url;
      // Form the assertion to sign.
      {
        std::string payloadStr;
        // Add GUID, current time, and expiration time to the payload
        {
          using Azure::Core::Uuid;
          using Azure::Core::_internal::PosixTimeConverter;

          std::ostringstream payloadStream;

          const Azure::DateTime now = std::chrono::system_clock::now();
          const Azure::DateTime exp = now + std::chrono::minutes(10);

          payloadStream << m_tokenPayloadStaticPart << Uuid::CreateUuid().ToString()
                        << "\",\"nbf\":" << PosixTimeConverter::DateTimeToPosixTime(now)
                        << ",\"exp\":" << PosixTimeConverter::DateTimeToPosixTime(exp) << "}";

          payloadStr = payloadStream.str();
        }

        static_assert(
            sizeof(std::string::value_type) == sizeof(uint8_t),
            "std::string::value_type value should be representable as uint8_t");

        // Concatenate JWT token header + "." + encoded payload
        const auto payloadVec
            = std::vector<std::string::value_type>(payloadStr.begin(), payloadStr.end());

        assertion += std::string(".")
            + Base64Url::Base64UrlEncode(reinterpret_cast<const std::vector<uint8_t>&>(payloadVec));
      }

      // Get assertion signature.
      std::string signature;
      if (auto mdCtx = EVP_MD_CTX_new())
      {
        try
        {
          EVP_PKEY_CTX* signCtx = nullptr;
          if ((EVP_DigestSignInit(
                   mdCtx, &signCtx, EVP_sha256(), nullptr, static_cast<EVP_PKEY*>(m_pkey))
               == 1)
              && (EVP_PKEY_CTX_set_rsa_padding(signCtx, RSA_PKCS1_PADDING) == 1))
          {
            size_t sigLen = 0;
            if (EVP_DigestSign(mdCtx, nullptr, &sigLen, nullptr, 0) == 1)
            {
              const auto bufToSign = reinterpret_cast<const unsigned char*>(assertion.data());
              const auto bufToSignLen = static_cast<size_t>(assertion.size());

              std::vector<unsigned char> sigVec(sigLen);
              if (EVP_DigestSign(mdCtx, sigVec.data(), &sigLen, bufToSign, bufToSignLen) == 1)
              {
                static_assert(
                    sizeof(unsigned char) == sizeof(uint8_t),
                    "unsigned char should be representable as uint8_t");

                signature = Base64Url::Base64UrlEncode(
                    reinterpret_cast<const std::vector<uint8_t>&>(sigVec));
              }
            }
          }

          if (signature.empty())
          {
            throw Azure::Core::Credentials::AuthenticationException(
                "Failed to sign token request.");
          }

          EVP_MD_CTX_free(mdCtx);
        }
        catch (...)
        {
          EVP_MD_CTX_free(mdCtx);
          throw;
        }
      }

      // Add signature to the end of assertion
      assertion += std::string(".") + signature;
    }

    body << "&client_assertion=" << Azure::Core::Url::Encode(assertion);

    auto request = std::make_unique<TokenCredentialImpl::TokenRequest>(
        HttpMethod::Post, m_requestUrl, body.str());

    return request;
  });
}

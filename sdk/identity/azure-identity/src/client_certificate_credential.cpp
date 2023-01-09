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
#include <vector>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

using Azure::Identity::ClientCertificateCredential;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Uuid;
using Azure::Core::_internal::Base64Url;
using Azure::Core::_internal::PosixTimeConverter;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::TokenCredentialImpl;

namespace {
template <typename T> std::vector<uint8_t> ToUInt8Vector(T const& in)
{
  const size_t size = in.size();
  std::vector<uint8_t> outVec(size);
  for (size_t i = 0; i < size; ++i)
  {
    outVec[i] = static_cast<uint8_t>(in[i]);
  }

  return outVec;
}

template <typename> struct UniqueHandleHelper;

template <> struct UniqueHandleHelper<BIO>
{
  using type = Azure::Core::_internal::BasicUniqueHandle<BIO, BIO_free_all>;
};

template <> struct UniqueHandleHelper<X509>
{
  using type = Azure::Core::_internal::BasicUniqueHandle<X509, X509_free>;
};

template <> struct UniqueHandleHelper<EVP_MD_CTX>
{
  using type = Azure::Core::_internal::BasicUniqueHandle<EVP_MD_CTX, EVP_MD_CTX_free>;
};

template <typename T>
using UniqueHandle = Azure::Core::_internal::UniqueHandle<T, UniqueHandleHelper<T>>;
} // namespace

void Azure::Identity::_detail::FreePkeyImpl(void* pkey)
{
  EVP_PKEY_free(static_cast<EVP_PKEY*>(pkey));
}

ClientCertificateCredential::ClientCertificateCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    std::string const& authorityHost,
    TokenCredentialOptions const& options)
    : m_clientCredentialCore(tenantId, authorityHost),
      m_tokenCredentialImpl(std::make_unique<TokenCredentialImpl>(options)),
      m_requestBody(
          std::string(
              "grant_type=client_credentials"
              "&client_assertion_type="
              "urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer" // cspell:disable-line
              "&client_id=")
          + Url::Encode(clientId)),
      m_tokenPayloadStaticPart(
          "\",\"iss\":\"" + clientId + "\",\"sub\":\"" + clientId + "\",\"jti\":\"")
{
  std::string thumbprintHexStr;
  std::string thumbprintBase64Str;

  {
    std::vector<unsigned char> mdVec(EVP_MAX_MD_SIZE);
    {
      UniqueHandle<X509> x509;
      {
        // Open certificate file, then get private key and X509:
        UniqueHandle<BIO> bio(BIO_new_file(clientCertificatePath.c_str(), "r"));
        if (!bio)
        {
          throw AuthenticationException("Failed to open certificate file.");
        }

        m_pkey.reset(PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr));
        if (!m_pkey)
        {
          throw AuthenticationException("Failed to read certificate private key.");
        }

        x509.reset(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
        if (!x509)
        {
          static_cast<void>(BIO_seek(bio.get(), 0));
          x509.reset(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
          if (!x509)
          {
            throw AuthenticationException("Failed to read X509 section.");
          }
        }
      }

      // Get certificate thumbprint:
      unsigned int mdLen = 0;
      const auto digestResult = X509_digest(x509.get(), EVP_sha1(), mdVec.data(), &mdLen);

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
    }

    // Get thumbprint as hex string:
    {
      std::ostringstream thumbprintStream;
      for (const auto md : mdVec)
      {
        thumbprintStream << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
                         << static_cast<int>(md);
      }
      thumbprintHexStr = thumbprintStream.str();
    }

    // Get thumbprint as Base64:
    thumbprintBase64Str = Base64Url::Base64UrlEncode(ToUInt8Vector(mdVec));
  }

  // Form a JWT token:
  const auto tokenHeader = std::string("{\"x5t\":\"") + thumbprintBase64Str + "\",\"kid\":\""
      + thumbprintHexStr + "\",\"alg\":\"RS256\",\"typ\":\"JWT\"}";

  const auto tokenHeaderVec
      = std::vector<std::string::value_type>(tokenHeader.begin(), tokenHeader.end());

  m_tokenHeaderEncoded = Base64Url::Base64UrlEncode(ToUInt8Vector(tokenHeaderVec));
}

ClientCertificateCredential::ClientCertificateCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    ClientCertificateCredentialOptions const& options)
    : ClientCertificateCredential(
        tenantId,
        clientId,
        clientCertificatePath,
        options.AuthorityHost,
        options)
{
}

ClientCertificateCredential::ClientCertificateCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    TokenCredentialOptions const& options)
    : ClientCertificateCredential(
        tenantId,
        clientId,
        clientCertificatePath,
        ClientCertificateCredentialOptions{}.AuthorityHost,
        options)
{
}

ClientCertificateCredential::~ClientCertificateCredential() = default;

AccessToken ClientCertificateCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const scopesStr = m_clientCredentialCore.GetScopesString(tokenRequestContext.Scopes);

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      auto body = m_requestBody;
      if (!scopesStr.empty())
      {
        body += "&scope=" + scopesStr;
      }

      auto const requestUrl = m_clientCredentialCore.GetRequestUrl();

      std::string assertion = m_tokenHeaderEncoded;
      {
        // Form the assertion to sign.
        {
          std::string payloadStr;
          // Add GUID, current time, and expiration time to the payload
          {
            // MSAL has JWT token expiration hardcoded as 10 minutes, without further explanations
            // anywhere nearby the constant.
            // https://github.com/AzureAD/microsoft-authentication-library-for-dotnet/blob/01ecd12464007fc1988b6a127aa0b1b980bca1ed/src/client/Microsoft.Identity.Client/Internal/JsonWebTokenConstants.cs#L8
            DateTime const now = std::chrono::system_clock::now();
            DateTime const exp = now + std::chrono::minutes(10);

            payloadStr = std::string("{\"aud\":\"") + requestUrl.GetAbsoluteUrl()
                + m_tokenPayloadStaticPart + Uuid::CreateUuid().ToString()
                + "\",\"nbf\":" + std::to_string(PosixTimeConverter::DateTimeToPosixTime(now))
                + ",\"exp\":" + std::to_string(PosixTimeConverter::DateTimeToPosixTime(exp)) + "}";
          }

          // Concatenate JWT token header + "." + encoded payload
          const auto payloadVec
              = std::vector<std::string::value_type>(payloadStr.begin(), payloadStr.end());

          assertion += std::string(".") + Base64Url::Base64UrlEncode(ToUInt8Vector(payloadVec));
        }

        // Get assertion signature.
        std::string signature;
        {
          UniqueHandle<EVP_MD_CTX> mdCtx(EVP_MD_CTX_new());
          if (mdCtx)
          {
            EVP_PKEY_CTX* signCtx = nullptr;
            if ((EVP_DigestSignInit(
                     mdCtx.get(),
                     &signCtx,
                     EVP_sha256(),
                     nullptr,
                     static_cast<EVP_PKEY*>(m_pkey.get()))
                 == 1)
                && (EVP_PKEY_CTX_set_rsa_padding(signCtx, RSA_PKCS1_PADDING) == 1))
            {
              size_t sigLen = 0;
              if (EVP_DigestSign(mdCtx.get(), nullptr, &sigLen, nullptr, 0) == 1)
              {
                const auto bufToSign = reinterpret_cast<const unsigned char*>(assertion.data());
                const auto bufToSignLen = static_cast<size_t>(assertion.size());

                std::vector<unsigned char> sigVec(sigLen);
                if (EVP_DigestSign(mdCtx.get(), sigVec.data(), &sigLen, bufToSign, bufToSignLen)
                    == 1)
                {
                  signature = Base64Url::Base64UrlEncode(ToUInt8Vector(sigVec));
                }
              }
            }
          }
        }

        if (signature.empty())
        {
          throw Azure::Core::Credentials::AuthenticationException("Failed to sign token request.");
        }

        // Add signature to the end of assertion
        assertion += std::string(".") + signature;
      }

      body += "&client_assertion=" + Azure::Core::Url::Encode(assertion);

      auto request
          = std::make_unique<TokenCredentialImpl::TokenRequest>(HttpMethod::Post, requestUrl, body);

      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());

      return request;
    });
  });
}

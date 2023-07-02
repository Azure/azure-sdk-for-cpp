// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/client_certificate_credential.hpp"

#include "private/tenant_id_resolver.hpp"
#include "private/token_credential_impl.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/core/uuid.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#include <wil/resource.h>
#include <wil/result.h>
#else
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#endif

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
using Azure::Identity::_detail::TenantIdResolver;
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

using CertificateThumbprint = std::vector<unsigned char>;
using UniquePrivateKey = Azure::Identity::_detail::UniquePrivateKey;
using PrivateKey = decltype(std::declval<UniquePrivateKey>().get());

#ifdef WIN32
CertificateThumbprint GetThumbprint(PCCERT_CONTEXT cert)
{
  DWORD size = 0;
  THROW_IF_WIN32_BOOL_FALSE_MSG(
      CertGetCertificateContextProperty(cert, CERT_SHA1_HASH_PROP_ID, nullptr, &size),
      "Failed to get certificate thumbprint.");
  std::vector<unsigned char> thumbprint(size);
  THROW_IF_WIN32_BOOL_FALSE_MSG(
      CertGetCertificateContextProperty(cert, CERT_SHA1_HASH_PROP_ID, thumbprint.data(), &size),
      "Failed to get certificate thumbprint.");
  return thumbprint;
}

UniquePrivateKey GetPrivateKey(PCCERT_CONTEXT cert)
{
  NCRYPT_KEY_HANDLE key;
  DWORD size = sizeof(void*);
  THROW_IF_WIN32_BOOL_FALSE_MSG(
      CertGetCertificateContextProperty(
          cert, CERT_NCRYPT_KEY_HANDLE_PROP_ID, &key, &size),
      "Failed to get certificate private key.");
  return UniquePrivateKey{key};
}

std::tuple<CertificateThumbprint, UniquePrivateKey> ReadCertificate(const std::string& path)
{
  wil::unique_hcertstore certStore;
  wil::unique_hcryptmsg certMsg;
  wil::unique_cert_context cert;
  std::wstring pathw{path.begin(), path.end()};
  DWORD encodingType, contentType, formatType;
  THROW_IF_WIN32_BOOL_FALSE_MSG(
      CryptQueryObject(
          CERT_QUERY_OBJECT_FILE,
          pathw.c_str(),
          CERT_QUERY_CONTENT_CERT | CERT_QUERY_CONTENT_SERIALIZED_CERT,
          CERT_QUERY_FORMAT_FLAG_ALL,
          0,
          &encodingType,
          &contentType,
          &formatType,
          wil::out_param(certStore),
          wil::out_param(certMsg),
          wil::out_param_ptr<const void**>(cert)),
      "Failed to open certificate file.");
  return std::make_tuple(GetThumbprint(cert.get()), GetPrivateKey(cert.get()));
}

std::vector<unsigned char> SignPkcs1Sha256(PrivateKey key, const uint8_t* data, size_t size)
{
  auto hash = Azure::Core::Cryptography::_internal::Sha256Hash().Final(data, size);
  BCRYPT_PKCS1_PADDING_INFO paddingInfo;
  paddingInfo.pszAlgId = BCRYPT_SHA256_ALGORITHM;
  DWORD signatureSize = 0;
  auto status = NCryptSignHash(
      key,
      &paddingInfo,
      hash.data(),
      static_cast<ULONG>(hash.size()),
      nullptr,
      0,
      &signatureSize,
      BCRYPT_PAD_PKCS1);
  THROW_HR_IF(status, status != NTE_BUFFER_TOO_SMALL);
  std::vector<unsigned char> signature(signatureSize);
  status = NCryptSignHash(
      key,
      &paddingInfo,
      hash.data(),
      static_cast<ULONG>(hash.size()),
      signature.data(),
      static_cast<ULONG>(signature.size()),
      &signatureSize,
      BCRYPT_PAD_PKCS1);
  THROW_HR_IF(status, status != ERROR_SUCCESS);
  return signature;
}
#else
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
using UniqueHandle = Azure::Core::_internal::UniqueHandle<T, UniqueHandleHelper>;

std::tuple<CertificateThumbprint, UniquePrivateKey> ReadCertificate(const std::string& path)
{
  // Open certificate file, then get private key and X509:
  UniqueHandle<BIO> bio(BIO_new_file(path.c_str(), "r"));
  if (!bio)
  {
    throw AuthenticationException("Failed to open certificate file.");
  }

  UniquePrivateKey pkey{PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr)};
  if (!pkey)
  {
    throw AuthenticationException("Failed to read certificate private key.");
  }

  UniqueHandle<X509> x509{PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr)};
  if (!x509)
  {
    std::ignore = BIO_seek(bio.get(), 0);
    x509.reset(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
    if (!x509)
    {
      throw AuthenticationException("Failed to read X509 section.");
    }
  }

  CertificateThumbprint thumbprint(EVP_MAX_MD_SIZE);
  // Get certificate thumbprint:
  unsigned int mdLen = 0;
  const auto digestResult = X509_digest(x509.get(), EVP_sha1(), thumbprint.data(), &mdLen);

  if (!digestResult)
  {
    throw AuthenticationException("Failed to get certificate thumbprint.");
  }

  // Drop unused buffer space:
  const auto mdLenSz = static_cast<decltype(thumbprint)::size_type>(mdLen);
  if (thumbprint.size() > mdLenSz)
  {
    thumbprint.resize(mdLenSz);
  }

  return std::make_tuple(thumbprint, std::move(pkey));
}

std::vector<unsigned char> SignPkcs1Sha256(PrivateKey key, const uint8_t* data, size_t size)
{
  UniqueHandle<EVP_MD_CTX> mdCtx(EVP_MD_CTX_new());
  if (!mdCtx)
  {
    return {};
  }
  EVP_PKEY_CTX* signCtx = nullptr;
  if ((EVP_DigestSignInit(mdCtx.get(), &signCtx, EVP_sha256(), nullptr, static_cast<EVP_PKEY*>(key))
       == 1)
      && (EVP_PKEY_CTX_set_rsa_padding(signCtx, RSA_PKCS1_PADDING) == 1))
  {
    size_t sigLen = 0;
    if (EVP_DigestSign(mdCtx.get(), nullptr, &sigLen, nullptr, 0) == 1)
    {
      std::vector<unsigned char> sigVec(sigLen);
      if (EVP_DigestSign(mdCtx.get(), sigVec.data(), &sigLen, data, size) == 1)
      {
        return sigVec;
      }
    }
  }
  return {};
}
#endif
} // namespace

#ifdef WIN32
void Azure::Identity::_detail::FreeNcryptKeyImpl(void* pkey)
{
  NCryptFreeObject(reinterpret_cast<NCRYPT_KEY_HANDLE>(pkey));
}
#else
void Azure::Identity::_detail::FreePkeyImpl(void* pkey)
{
  EVP_PKEY_free(static_cast<EVP_PKEY*>(pkey));
}
#endif

ClientCertificateCredential::ClientCertificateCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    std::string const& authorityHost,
    std::vector<std::string> additionallyAllowedTenants,
    Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("ClientCertificateCredential"),
      m_clientCredentialCore(tenantId, authorityHost, additionallyAllowedTenants),
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
    CertificateThumbprint mdVec;
    try
    {
      std::tie(mdVec, m_pkey) = ReadCertificate(clientCertificatePath);
    }
    catch (AuthenticationException&)
    {
      throw;
    }
    catch (std::exception& e)
    {
      // WIL does not throw AuthenticationException.
      throw AuthenticationException(e.what());
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
        options.AdditionallyAllowedTenants,
        options)
{
}

ClientCertificateCredential::ClientCertificateCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientCertificatePath,
    Core::Credentials::TokenCredentialOptions const& options)
    : ClientCertificateCredential(
        tenantId,
        clientId,
        clientCertificatePath,
        ClientCertificateCredentialOptions{}.AuthorityHost,
        ClientCertificateCredentialOptions{}.AdditionallyAllowedTenants,
        options)
{
}

ClientCertificateCredential::~ClientCertificateCredential() = default;

AccessToken ClientCertificateCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const tenantId = TenantIdResolver::Resolve(
      m_clientCredentialCore.GetTenantId(),
      tokenRequestContext,
      m_clientCredentialCore.GetAdditionallyAllowedTenants());

  auto const scopesStr
      = m_clientCredentialCore.GetScopesString(tenantId, tokenRequestContext.Scopes);

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tenantId, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      auto body = m_requestBody;
      if (!scopesStr.empty())
      {
        body += "&scope=" + scopesStr;
      }

      auto const requestUrl = m_clientCredentialCore.GetRequestUrl(tenantId);

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
        std::string signature = Base64Url::Base64UrlEncode(SignPkcs1Sha256(
            m_pkey.get(),
            reinterpret_cast<const unsigned char*>(assertion.data()),
            static_cast<size_t>(assertion.size())));

        if (signature.empty())
        {
          throw AuthenticationException("Failed to sign token request.");
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

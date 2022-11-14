//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/cryptography/hash.hpp>
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <azure/keyvault/shared/keyvault_shared.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"
#include "azure/keyvault/keys/key_client_models.hpp"

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "../private/key_serializers.hpp"
#include "../private/key_sign_parameters.hpp"
#include "../private/key_verify_parameters.hpp"
#include "../private/key_wrap_parameters.hpp"
#include "../private/keyvault_protocol.hpp"
#include "../private/package_version.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

namespace {
// 1Mb at a time
const size_t DefaultStreamDigestReadSize = 1024 * 1024;

inline std::vector<uint8_t> CreateDigest(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data)
{
  // Use heap for the reading buffer.
  auto heapBuffer = std::make_unique<std::vector<uint8_t>>(DefaultStreamDigestReadSize);
  auto* buffer = heapBuffer.get()->data();
  auto hashAlgorithm = algorithm.GetHashAlgorithm();
  for (size_t read = data.Read(buffer, DefaultStreamDigestReadSize); read > 0;
       read = data.Read(buffer, DefaultStreamDigestReadSize))
  {
    hashAlgorithm->Append(buffer, read);
  }
  return hashAlgorithm->Final();
}

inline std::vector<uint8_t> CreateDigest(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data)
{
  auto hashAlgorithm = algorithm.GetHashAlgorithm();
  return hashAlgorithm->Final(data.data(), data.size());
}

} // namespace

Request CryptographyClient::CreateRequest(
    HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultKeysCommonRequest::CreateRequest(
      m_keyId, m_apiVersion, method, path, content);
}

std::unique_ptr<Azure::Core::Http::RawResponse> CryptographyClient::SendCryptoRequest(
    std::vector<std::string> const& path,
    std::string const& payload,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(HttpMethod::Post, path, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  request.SetHeader(HttpShared::Accept, HttpShared::ApplicationJson);

  // Send, parse and validate respone
  return Azure::Security::KeyVault::_detail::KeyVaultKeysCommonRequest::SendRequest(
      *m_pipeline, request, context);
}

CryptographyClient::~CryptographyClient() = default;

CryptographyClient::CryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions const& options)
    : m_keyId(Azure::Core::Url(keyId)), m_apiVersion(options.Version)
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{_internal::UrlScope::GetScopeFromUrl(m_keyId)}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      "KeyVault",
      PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

Azure::Response<EncryptResult> CryptographyClient::Encrypt(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {EncryptValue}, EncryptParametersSerializer::EncryptParametersSerialize(parameters), context);
  auto value = EncryptResultSerializer::EncryptResultDeserialize(*rawResponse);
  value.Algorithm = parameters.Algorithm;
  return Azure::Response<EncryptResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<DecryptResult> CryptographyClient::Decrypt(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {DecryptValue}, DecryptParametersSerializer::DecryptParametersSerialize(parameters), context);
  auto value = DecryptResultSerializer::DecryptResultDeserialize(*rawResponse);
  value.Algorithm = parameters.Algorithm;
  return Azure::Response<DecryptResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<WrapResult> CryptographyClient::WrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {WrapKeyValue},
      KeyWrapParametersSerializer::KeyWrapParametersSerialize(
          KeyWrapParameters(algorithm.ToString(), key)),
      context);
  auto value = WrapResultSerializer::WrapResultDeserialize(*rawResponse);
  value.Algorithm = algorithm;
  return Azure::Response<WrapResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<UnwrapResult> CryptographyClient::UnwrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& encryptedKey,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {UnwrapKeyValue},
      KeyWrapParametersSerializer::KeyWrapParametersSerialize(
          KeyWrapParameters(algorithm.ToString(), encryptedKey)),
      context);
  auto value = UnwrapResultSerializer::UnwrapResultDeserialize(*rawResponse);
  value.Algorithm = algorithm;
  return Azure::Response<UnwrapResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<SignResult> CryptographyClient::Sign(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& digest,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {SignValue},
      KeySignParametersSerializer::KeySignParametersSerialize(
          KeySignParameters(algorithm.ToString(), digest)),
      context);
  auto value = SignResultSerializer::SignResultDeserialize(*rawResponse);
  value.Algorithm = algorithm;
  return Azure::Response<SignResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<SignResult> CryptographyClient::SignData(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data,
    Azure::Core::Context const& context)
{
  return Sign(algorithm, CreateDigest(algorithm, data), context);
}

Azure::Response<SignResult> CryptographyClient::SignData(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data,
    Azure::Core::Context const& context)
{
  return Sign(algorithm, CreateDigest(algorithm, data), context);
}

Azure::Response<VerifyResult> CryptographyClient::Verify(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& digest,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  // Send and parse respone
  auto rawResponse = SendCryptoRequest(
      {VerifyValue},
      KeyVerifyParametersSerializer::KeyVerifyParametersSerialize(
          KeyVerifyParameters(algorithm.ToString(), digest, signature)),
      context);
  auto value = VerifyResultSerializer::VerifyResultDeserialize(*rawResponse);
  value.Algorithm = algorithm;
  value.KeyId = this->m_keyId.GetAbsoluteUrl();
  return Azure::Response<VerifyResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<VerifyResult> CryptographyClient::VerifyData(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  return Verify(algorithm, CreateDigest(algorithm, data), signature, context);
}

Azure::Response<VerifyResult> CryptographyClient::VerifyData(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  return Verify(algorithm, CreateDigest(algorithm, data), signature, context);
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/azure_assert.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"
#include "azure/keyvault/keys/key_operation.hpp"

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "../private/key_serializers.hpp"
#include "../private/key_sign_parameters.hpp"
#include "../private/key_verify_parameters.hpp"
#include "../private/key_wrap_parameters.hpp"
#include "../private/keyvault_protocol.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

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

CryptographyClient::~CryptographyClient() = default;

CryptographyClient::CryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions const& options)
{
  auto apiVersion = options.Version.ToString();
  m_keyId = Azure::Core::Url(keyId);
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient>(
      m_keyId,
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<EncryptResult> CryptographyClient::Encrypt(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  return m_pipeline->SendRequest<EncryptResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&parameters]() {
        return EncryptParametersSerializer::EncryptParametersSerialize(parameters);
      },
      [&parameters](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = EncryptResultSerializer::EncryptResultDeserialize(rawResponse);
        result.Algorithm = parameters.Algorithm;
        return result;
      },
      {"encrypt"});
}

Azure::Response<DecryptResult> CryptographyClient::Decrypt(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  return m_pipeline->SendRequest<DecryptResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&parameters]() {
        return DecryptParametersSerializer::DecryptParametersSerialize(parameters);
      },
      [&parameters](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = DecryptResultSerializer::DecryptResultDeserialize(rawResponse);
        result.Algorithm = parameters.Algorithm;
        return result;
      },
      {"decrypt"});
}

Azure::Response<WrapResult> CryptographyClient::WrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context)
{
  return m_pipeline->SendRequest<WrapResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &key]() {
        return KeyWrapParametersSerializer::KeyWrapParametersSerialize(
            KeyWrapParameters(algorithm.ToString(), key));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = WrapResultSerializer::WrapResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"wrapKey"});
}

Azure::Response<UnwrapResult> CryptographyClient::UnwrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& encryptedKey,
    Azure::Core::Context const& context)
{
  return m_pipeline->SendRequest<UnwrapResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &encryptedKey]() {
        return KeyWrapParametersSerializer::KeyWrapParametersSerialize(
            KeyWrapParameters(algorithm.ToString(), encryptedKey));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = UnwrapResultSerializer::UnwrapResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"unwrapKey"});
}

Azure::Response<SignResult> CryptographyClient::Sign(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& digest,
    Azure::Core::Context const& context)
{
  return m_pipeline->SendRequest<SignResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &digest]() {
        return KeySignParametersSerializer::KeySignParametersSerialize(
            KeySignParameters(algorithm.ToString(), digest));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = SignResultSerializer::SignResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"sign"});
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
  return m_pipeline->SendRequest<VerifyResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &digest, &signature]() {
        return KeyVerifyParametersSerializer::KeyVerifyParametersSerialize(
            KeyVerifyParameters(algorithm.ToString(), digest, signature));
      },
      [&algorithm, this](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = VerifyResultSerializer::VerifyResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        // Verify result won't return the KeyId, the client SDK will add it based on the client
        // KeyId.
        result.KeyId = this->m_keyId.GetAbsoluteUrl();
        return result;
      },
      {"verify"});
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

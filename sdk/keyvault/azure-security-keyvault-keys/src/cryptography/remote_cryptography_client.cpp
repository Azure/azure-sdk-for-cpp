// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "../private/key_serializers.hpp"
#include "../private/key_sign_parameters.hpp"
#include "../private/key_verify_parameters.hpp"
#include "../private/key_wrap_parameters.hpp"
#include "azure/keyvault/keys/internal/cryptography/remote_cryptography_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

RemoteCryptographyClient::RemoteCryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions options)
{
  auto apiVersion = options.Version.ToString();
  // Remote client is init with the URL to a key vault key.
  KeyId = Azure::Core::Url(keyId);
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  Pipeline = std::make_shared<Azure::Security::KeyVault::_internal::KeyVaultProtocolClient>(
      Azure::Core::Url(keyId),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultKey> RemoteCryptographyClient::GetKey(
    Azure::Core::Context const& context) const
{
  // The remote crypto client is created with a Key Vault key URL, hence, no path is required to get
  // the key from the server.
  return Pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return Azure::Security::KeyVault::Keys::_detail::KeyVaultKeySerializer::
            KeyVaultKeyDeserialize(rawResponse);
      },
      {});
}

Azure::Response<EncryptResult> RemoteCryptographyClient::EncryptWithResponse(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<EncryptResult>(
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

EncryptResult RemoteCryptographyClient::Encrypt(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return EncryptWithResponse(parameters, context).Value;
}

Azure::Response<DecryptResult> RemoteCryptographyClient::DecryptWithResponse(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<DecryptResult>(
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

DecryptResult RemoteCryptographyClient::Decrypt(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return DecryptWithResponse(parameters, context).Value;
}

Azure::Response<WrapResult> RemoteCryptographyClient::WrapKeyWithResponse(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<WrapResult>(
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

WrapResult RemoteCryptographyClient::WrapKey(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return WrapKeyWithResponse(algorithm, key, context).Value;
}

Azure::Response<UnwrapResult> RemoteCryptographyClient::UnwrapKeyWithResponse(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<UnwrapResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &key]() {
        return KeyWrapParametersSerializer::KeyWrapParametersSerialize(
            KeyWrapParameters(algorithm.ToString(), key));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = UnwrapResultSerializer::UnwrapResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"unwrapKey"});
}

UnwrapResult RemoteCryptographyClient::UnwrapKey(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return UnwrapKeyWithResponse(algorithm, key, context).Value;
}

Azure::Response<SignResult> RemoteCryptographyClient::SignWithResponse(
    SignatureAlgorithm const& algorithm,
    std::vector<uint8_t> const& digest,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<SignResult>(
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

SignResult RemoteCryptographyClient::Sign(
    SignatureAlgorithm const& algorithm,
    std::vector<uint8_t> const& digest,
    Azure::Core::Context const& context) const
{
  return SignWithResponse(algorithm, digest, context).Value;
}

Azure::Response<VerifyResult> RemoteCryptographyClient::VerifyWithResponse(
    SignatureAlgorithm const& algorithm,
    std::vector<uint8_t> const& digest,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<VerifyResult>(
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
        result.KeyId = this->KeyId.GetAbsoluteUrl();
        return result;
      },
      {"verify"});
}

VerifyResult RemoteCryptographyClient::Verify(
    SignatureAlgorithm const& algorithm,
    std::vector<uint8_t> const& digest,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context) const
{
  return VerifyWithResponse(algorithm, digest, signature, context).Value;
}

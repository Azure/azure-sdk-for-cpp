// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_request_parameters.hpp"
#include "azure/keyvault/keys/key_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Http;

KeyClient::KeyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::TokenCredential const> credential,
    KeyClientOptions options)
{
  auto apiVersion = options.GetVersionString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Http::TokenRequestOptions const tokenOptions
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenOptions));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline>(
      Azure::Core::Url(vaultUrl),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, options.Version});
}

Azure::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    JsonWebKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(keyType, options),
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = ecKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(ecKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = rsaKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(rsaKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = octKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(octKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation KeyClient::StartDeleteKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      m_pipeline,
      m_pipeline->SendRequest<Azure::Security::KeyVault::Keys::DeletedKey>(
          context,
          Azure::Core::Http::HttpMethod::Delete,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::DeletedKeyDeserialize(name, rawResponse);
          },
          {_detail::KeysPath, name}));
}

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

  // Base Pipeline
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  policies.emplace_back(
      std::make_unique<TelemetryPolicy>("KeyVault", apiVersion, options.TelemetryPolicyOptions));
  policies.emplace_back(std::make_unique<RequestIdPolicy>());
  policies.emplace_back(std::make_unique<RetryPolicy>(options.RetryOptions));

  {
    Azure::Core::Http::TokenRequestOptions const tokenOptions
        = {{"https://vault.azure.net/.default"}};

    policies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenOptions));
  }

  policies.emplace_back(std::make_unique<LoggingPolicy>(options.LoggingPolicyOptions));
  policies.emplace_back(
      std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
  Azure::Core::Http::Url url(vaultUrl);

  m_pipeline = std::make_shared<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline>(
      url, apiVersion, std::move(policies));
}

Azure::Core::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return Details::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {Details::KeysPath, name, options.Version});
}

Azure::Core::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    JsonWebKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      Details::KeyRequestParameters(keyType, options),
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return Details::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {Details::KeysPath, name, "create"});
}

Azure::Core::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = ecKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      Details::KeyRequestParameters(ecKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return Details::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {Details::KeysPath, keyName, "create"});
}

Azure::Core::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = rsaKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      Details::KeyRequestParameters(rsaKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return Details::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {Details::KeysPath, keyName, "create"});
}

Azure::Core::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = octKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      Details::KeyRequestParameters(octKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return Details::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {Details::KeysPath, keyName, "create"});
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
            return Details::DeletedKeyDeserialize(name, rawResponse);
          },
          {Details::KeysPath, name}));
}

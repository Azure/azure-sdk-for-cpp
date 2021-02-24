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
      std::make_unique<TelemetryPolicy>("KeyVault", apiVersion, options.Telemetry));
  policies.emplace_back(std::make_unique<RequestIdPolicy>());
  policies.emplace_back(std::make_unique<RetryPolicy>(options.Retry));

  {
    Azure::Core::Http::TokenRequestOptions const tokenOptions
        = {{"https://vault.azure.net/.default"}};

    policies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenOptions));
  }

  policies.emplace_back(std::make_unique<LoggingPolicy>());
  policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(options.Transport));
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

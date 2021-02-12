// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>

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

  policies.emplace_back(std::make_unique<LoggingPolicy>());
  policies.emplace_back(
      std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
  Azure::Core::Http::Url url(vaultUrl);

  m_pipeline = std::make_shared<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline>(
      url, apiVersion, std::move(policies));
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/cryptography/remote_cryptography_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

RemoteCryptographyClient::RemoteCryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions options)
{
  auto apiVersion = options.GetVersionString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline>(
      Azure::Core::Url(keyId),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

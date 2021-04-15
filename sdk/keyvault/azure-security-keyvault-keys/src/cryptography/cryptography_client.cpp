// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_internal;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

CryptographyClient::CryptographyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions const& options,
    bool forceRemote)
{
  auto apiVersion = options.Version.ToString();
  m_keyId = vaultUrl;
  m_remoteProvider = std::make_shared<RemoteCryptographyClient>(vaultUrl, credential, options);
  m_pipeline = m_remoteProvider->Pipeline;

  if (forceRemote)
  {
    m_provider = m_remoteProvider;
  }
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_provider.hpp"
#include "azure/keyvault/keys/cryptography/remote_cryptography_client.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  class CryptographyClient {
  private:
    std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> m_pipeline;
    std::string m_keyId;
    std::shared_ptr<
        Azure::Security::KeyVault::Keys::Cryptography::_internal::RemoteCryptographyClient>
        m_remoteProvider;
    std::shared_ptr<Azure::Security::KeyVault::Keys::Cryptography::_internal::CryptographyProvider>
        m_provider;

    explicit CryptographyClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions const& options,
        bool forceRemote);

  public:
    explicit CryptographyClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions options = CryptographyClientOptions())
        : CryptographyClient(vaultUrl, credential, options, false)
    {
    }

    std::shared_ptr<Azure::Security::KeyVault::Keys::Cryptography::_internal::CryptographyProvider>
    RemoteClient() const
    {
      return m_remoteProvider;
    }

    bool LocalOnly() const { return m_remoteProvider == nullptr; }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

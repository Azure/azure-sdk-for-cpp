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
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
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

    /**
     * @brief Encrypts plaintext.
     *
     * @param parameters An #EncryptParameters containing the data to encrypt and other parameters
     * for algorithm-dependent encryption.
     * @param context A #Azure::Core::Context to cancel the operation.
     * @return An #EncryptResult containing the encrypted data along with all other information
     * needed to decrypt it. This information should be stored with the encrypted data.
     */
    EncryptResult Encrypt(
        EncryptParameters parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      
    }

    /**
     * @brief Encrypts the specified plaintext.
     *
     * @param algorithm The <see cref="EncryptionAlgorithm"/> to use.</param>
     * @param plaintext The data to encrypt.
     * @param context A #Azure::Core::Context to cancel the operation.
     * @return An #EncryptResult containing the encrypted data along with all other information
     * needed to decrypt it. This information should be stored with the encrypted data.
     */
    EncryptResult Encrypt(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> const& plaintext,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      return Encrypt(EncryptParameters(algorithm, plaintext), context);
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

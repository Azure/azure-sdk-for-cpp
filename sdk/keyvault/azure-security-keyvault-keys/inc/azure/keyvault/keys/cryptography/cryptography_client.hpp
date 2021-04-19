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
        Azure::Security::KeyVault::Keys::Cryptography::_detail::RemoteCryptographyClient>
        m_remoteProvider;
    std::shared_ptr<Azure::Security::KeyVault::Keys::Cryptography::_detail::CryptographyProvider>
        m_provider;

    explicit CryptographyClient(
        std::string const& keyId,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions const& options,
        bool forceRemote);

    void Initialize(std::string const& operation, Azure::Core::Context const& context);

    void ThrowIfLocalOnly(std::string const& name)
    {
      if (LocalOnly())
      {
        throw std::invalid_argument(name + " Not supported.");
      }
    }

  public:
    /**
     * @brief Initializes a new instance of the #CryptographyClient class.
     *
     * @param keyId The key identifier of the #KeyVaultKey which will be used for cryptographic
     * operations.
     * @param credential A #TokenCredential used to authenticate requests to the vault, like
     * DefaultAzureCredential.
     * @param options #CryptographyClientOptions for the #CryptographyClient for local or remote
     * operations on Key Vault.
     */
    explicit CryptographyClient(
        std::string const& keyId,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions options = CryptographyClientOptions())
        : CryptographyClient(keyId, credential, options, false)
    {
    }

    std::shared_ptr<Azure::Security::KeyVault::Keys::Cryptography::_detail::CryptographyProvider>
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
        EncryptParameters const& parameters,
        Azure::Core::Context const& context = Azure::Core::Context());

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
        Azure::Core::Context const& context = Azure::Core::Context())
    {
      return Encrypt(EncryptParameters(algorithm, plaintext), context);
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

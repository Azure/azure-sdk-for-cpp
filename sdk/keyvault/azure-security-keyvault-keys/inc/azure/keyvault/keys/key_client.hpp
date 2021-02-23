// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Keys client.
 *
 */

#pragma once

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/delete_key_operation.hpp"
#include "azure/keyvault/keys/key_client_options.hpp"
#include "azure/keyvault/keys/key_create_options.hpp"
#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <functional>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The KeyClient provides synchronous methods to manage a KeyVaultKe in the Azure Key
   * Vault. The client supports creating, retrieving, updating, deleting, purging, backing up,
   * restoring, and listing the KeyVaultKey.
   */
  class KeyClient {1
  protected:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    std::shared_ptr<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline> m_pipeline;

  public:
    /**
     * @brief Construct a new Key Client object
     *
     * @param vaultUrl The url address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit KeyClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::TokenCredential const> credential,
        KeyClientOptions options = KeyClientOptions());

    /**
     * @brief Optional parameters for KeyVaultClient::GetKey
     *
     */
    struct GetKeyOptions
    {
      /**
       * @brief Context for cancelling long running operations.
       */
      Azure::Core::Context Context;
      /**
       * @brief Specify the key version to get.
       */
      std::string Version;
    };

    /**
     * @brief Gets the public part of a stored key.
     *
     * @remark The get key operation is applicable to all key types. If the requested key is
     * symmetric, then no key is released in the response. This operation requires the keys/get
     * permission.
     *
     * @param name The name of the key.
     * @param options Optional parameters for this operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> GetKey(
        std::string const& name,
        GetKeyOptions const& options = GetKeyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Creates and stores a new key in Key Vault. The create key operation can be used to
     * create any key type in Azure Key Vault. If the named key already exists, Azure Key Vault
     * creates a new version of the key. It requires the keys/create permission.
     *
     * @param name The name of the key.
     * @param keyType The type of key to create. See
     * #Azure::Security::KeyVault::Keys::JsonWebKeyType.
     * @param options Optional parameters for this operation. See
     * #Azure::Security::KeyVault::Keys::CreateKeyOptions.
     * @param context The context for the operation can be used for request cancellation.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> CreateKey(
        std::string const& name,
        JsonWebKeyType keyType,
        CreateKeyOptions const& options = CreateKeyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Creates and stores a new Elliptic Curve key in Key Vault.
     *
     * @remark If the named key already exists, Azure Key Vault creates a new version of the key.
     *
     * @remark This operation requires the keys/create permission.
     *
     * @param ecKeyOptions The key options object containing information about the Elliptic Curve
     * key being created.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> CreateEcKey(
        CreateEcKeyOptions ecKeyOptions,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Creates and stores a new RSA key in Key Vault.
     *
     * @remark If the named key already exists, Azure Key Vault creates a new version of the key.
     *
     * @remark This operation requires the keys/create permission.
     *
     * @param rsaKeyOptions The key options object containing information about the RSA key being
     * created.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> CreateRsaKey(
        CreateRsaKeyOptions rsaKeyOptions,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Creates and stores a new AES key in Key Vault.
     *
     * @remark If the named key already exists, Azure Key Vault creates a new version of the key.
     *
     * @remark This operation requires the keys/create permission.
     *
     * @param octKeyOptions The key options object containing information about the AES key being
     * created.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> CreateOctKey(
        CreateOctKeyOptions octKeyOptions,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes a key of any type from storage in Azure Key Vault.
     *
     * @remark The delete key operation cannot be used to remove individual versions of a key. This
     * operation removes the cryptographic material associated with the key, which means the key is
     * not usable for Sign/Verify, Wrap/Unwrap or Encrypt/Decrypt operations. This operation
     * requires the keys/delete permission.
     *
     * @param name The name of the key.
     * @param context A cancellation token controlling the request lifetime.
     * @return A #Azure::Security::KeyVault::Keys::DeleteKeyOperation to wait on this long-running
     * operation. If the key is soft delete-enabled, you only need to wait for the operation to
     * complete if you need to recover or purge the key; otherwise, the key is deleted automatically
     * on purge schedule.
     */
    Azure::Security::KeyVault::Keys::DeleteKeyOperation StartDeleteKey(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;
  };
}}}} // namespace Azure::Security::KeyVault::Keys

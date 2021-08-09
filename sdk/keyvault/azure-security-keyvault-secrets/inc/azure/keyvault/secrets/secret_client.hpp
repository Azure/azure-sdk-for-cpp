// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines the Key Vault Secret client.
 *
 */

#pragma once

#include "azure/keyvault/secrets/keyvault_backup_secret.hpp"
#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include "dll_import_export.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>

#include <stdint.h>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace _detail {
  class KeyVaultProtocolClient;
}}}} // namespace Azure::Security::KeyVault::_detail

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  class ServiceVersion final {
  private:
    std::string m_version;

  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for the Key Vault keys service.
     */
    ServiceVersion(std::string version) : m_version(std::move(version)) {}

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(ServiceVersion const& other) const { return m_version == other.m_version; }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_version; }

    /**
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    AZURE_SECURITY_KEYVAULT_SECRETS_DLLEXPORT static const ServiceVersion V7_2;
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct SecretClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;

    /**
     * @brief Construct a new Secret Client Options object.
     *
     * @param version Optional version for the client.
     */
    SecretClientOptions(ServiceVersion version = ServiceVersion::V7_2)
        : Azure::Core::_internal::ClientOptions(), Version(version)
    {
    }
  };

  /**
   * @brief Optional parameters for SecretClient::GetSecret
   *
   */
  struct GetSecretOptions final
  {
    /**
     * @brief Specify the secret version to get.
     *
     */
    std::string Version;
  };

  /**
   * @brief Optional parameters for SecretClient::UpdateSecretParameters
   *
   */
  struct UpdateSecretPropertiesOptions final
  {
    /**
     * @brief Specify the secret version to update.
     *
     */
    std::string Version;
  };

  /**
   * @brief Define a model for a purged key.
   *
   */
  struct PurgedSecret final
  {
  };

  /**
   * @brief The SecretClient provides synchronous methods to manage a secret in the Azure Key
   * Vault. The client supports creating, retrieving, updating, deleting, purging, backing up,
   * restoring, and listing the secret.
   */
  class SecretClient
#if !defined(TESTING_BUILD)
      final
#endif
  {

  protected:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    std::shared_ptr<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient> m_protocolClient;

  public:
    /**
     * @brief Construct a new SecretClient object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit SecretClient(
        std::string const& vaultUrl,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential const> credential,
        SecretClientOptions options = SecretClientOptions());

    /**
     * @brief Construct a new Key Client object from another key client.
     *
     * @param keyClient An existing key vault key client.
     */
    explicit SecretClient(SecretClient const& keyClient)
        : m_protocolClient(keyClient.m_protocolClient){};

    ~SecretClient() = default;

    /**
     * @brief Get a specified secret from a given key vault
     * This operation is applicable to any secret stored in Azure Key Vault.
     * This operation requires the secrets/get permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param options The optional parameters for this request.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> GetSecret(
        std::string const& name,
        GetSecretOptions const& options = GetSecretOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief The Get Deleted Secret operation returns
     * the specified deleted secret along with its attributes.
     * This operation requires the secrets/get permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultDeletedSecret> GetDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Set a secret in a specified key vault.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param value The value of the secret<span class="x x-first x-last">.</span>
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> SetSecret(
        std::string const& name,
        std::string const& value,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Set a secret in a specified key vault.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param secret The secret definition <span class="x x-first x-last">.</span>
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> SetSecret(
        std::string const& name,
        KeyVaultSecret const& secret,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Updates the attributes associated with a specified secret in a given key vault.
     * The UPDATE operation changes specified attributes of an existing stored secret.
     * Attributes that are not specified in the request are left unchanged.
     * The value of a secret itself cannot be changed.
     * This operation requires the secrets/set permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param options The optional parameters for this request.
     * @param properties The properties to update
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> UpdateSecretProperties(
        std::string const& name,
        UpdateSecretPropertiesOptions const& options,
        KeyvaultSecretProperties const& properties,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Update the attributes associated with a specified secret in a given key vault.
     * The UPDATE operation changes specified attributes of an existing stored secret.
     * Attributes that are not specified in the request are left unchanged.
     * The value of a secret itself cannot be changed.
     * This operation requires the secrets/set permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param version The version of the secret for this request.
     * @param properties The properties to update
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> UpdateSecretProperties(
        std::string const& name,
        std::string const& version,
        KeyvaultSecretProperties const& properties,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Back up the specified secret.
     * Requests that a backup of the specified secret be downloaded to the client.
     * All versions of the secret will be downloaded.
     * This operation requires the secrets/backup permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The The backup blob containing the backed up secret.
     */
    Azure::Response<BackupSecretResult> BackupSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Restore a backed up secret to a vault.
     * Restores a backed up secret, and all its versions, to a vault.
     * This operation requires the secrets/restore permission.
     *
     * @param backup The backup payload as encoded vector of bytes.
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> RestoreSecretBackup(
        std::vector<uint8_t> const& backup,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Permanently deletes the specified secret.
     * The purge deleted secret operation removes the secret permanently, without the possibility of
     * recovery. This operation can only be enabled on a soft-delete enabled vault. This operation
     * requires the secrets/purge permission.
     *
     * @param name The name of the secret<span class="x x-first x-last">.</span>
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return Response<PurgedSecret> is success.
     */
    Azure::Response<PurgedSecret> PurgeDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;
  };

}}}} // namespace Azure::Security::KeyVault::Secrets

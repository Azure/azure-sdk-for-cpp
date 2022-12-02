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
#include "azure/keyvault/secrets/keyvault_operations.hpp"
#include "azure/keyvault/secrets/keyvault_options.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret_paged_response.hpp"
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

  private:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    Azure::Core::Url m_vaultUrl;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

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
        : m_vaultUrl(keyClient.m_vaultUrl), m_apiVersion(keyClient.m_apiVersion),
          m_pipeline(keyClient.m_pipeline)

    {
    }

    ~SecretClient() = default;

    /**
     * @brief Get a specified secret from a given key vault
     * This operation is applicable to any secret stored in Azure Key Vault.
     * This operation requires the secrets/get permission.
     *
     * @param name The name of the secret.
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
     * @param name The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<DeletedSecret> GetDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Set a secret in a specified key vault.
     *
     * @param name The name of the secret.
     * @param value The value of the secret.
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
     * @param name The name of the secret.
     * @param secret The secret definition.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> SetSecret(
        std::string const& name,
        KeyVaultSecret const& secret,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Update the attributes associated with a specified secret in a given key vault.
     * The UPDATE operation changes specified attributes of an existing stored secret.
     * Attributes that are not specified in the request are left unchanged.
     * The value of a secret itself cannot be changed.
     * This operation requires the secrets/set permission.
     *
     * @param properties The properties to update
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> UpdateSecretProperties(
        SecretProperties const& properties,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Back up the specified secret.
     * Requests that a backup of the specified secret be downloaded to the client.
     * All versions of the secret will be downloaded.
     * This operation requires the secrets/backup permission.
     *
     * @param name The name of the secret.
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
        BackupSecretResult const& backup,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Permanently deletes the specified secret.
     * The purge deleted secret operation removes the secret permanently, without the possibility of
     * recovery. This operation can only be enabled on a soft-delete enabled vault. This operation
     * requires the secrets/purge permission.
     *
     * @param name The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return Response<PurgedSecret> is success.
     */
    Azure::Response<PurgedSecret> PurgeDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Delete a secret from a specified key vault.
     *
     * @remark The DELETE operation applies to any secret stored in Azure Key Vault.
     * DELETE cannot be applied to an individual version of a secret.
     * This operation requires the secrets/delete permission.
     *
     * @param name The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     */
    Azure::Security::KeyVault::Secrets::DeleteSecretOperation StartDeleteSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Recover the deleted secret to the latest version.
     *
     * @remark Recovers the deleted secret in the specified vault.
     * This operation can only be performed on a soft-delete enabled vault.
     * This operation requires the secrets/recover permission.
     *
     * @param name The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     */
    Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation StartRecoverDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List secrets in a specified key vault.
     * The Get Secrets operation is applicable to the entire vault.
     * However, only the base secret identifier and its attributes are provided in the response.
     * Individual secret versions are not listed in the response. This operation requires the
     * secrets/list permission.
     *
     * @param options The optional parameters for this request
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return Response containing a list of secrets in the vault along with a link to the next page
     * of secrets.
     */
    SecretPropertiesPagedResponse GetPropertiesOfSecrets(
        GetPropertiesOfSecretsOptions const& options = GetPropertiesOfSecretsOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List all versions of the specified secret.
     * The full secret identifier and attributes are provided in the response. No values are
     * returned for the secrets. This operations requires the secrets/list permission.
     *
     * @param name The name of the secret.
     * @param options The optional parameters for this request
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return Response containing a list of secrets in the vault along with a link to the next page
     * of secrets.
     */
    SecretPropertiesPagedResponse GetPropertiesOfSecretsVersions(
        std::string const& name,
        GetPropertiesOfSecretVersionsOptions const& options
        = GetPropertiesOfSecretVersionsOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Lists deleted secrets for the specified vault.
     * The Get Deleted Secrets operation returns the secrets that have been deleted for a vault
     * enabled for soft-delete. This operation requires the secrets/list permission.
     *
     * @param options The optional parameters for this request
     * @param context The context for the operation can be used for request cancellation.
     *
     * @return Response containing a list of deleted secrets in the vault, along with a link to the
     * next page of deleted secrets.
     */
    DeletedSecretPagedResponse GetDeletedSecrets(
        GetDeletedSecretsOptions const& options = GetDeletedSecretsOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the secret client's primary URL endpoint.
     *
     * @return The key secret's primary URL endpoint.
     */
    std::string GetUrl() const;

  private:
    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path = {},
        Azure::Core::IO::BodyStream* content = nullptr) const;
    Azure::Core::Http::Request ContinuationTokenRequest(
        std::vector<std::string> const& path,
        const Azure::Nullable<std::string>& NextPageToken) const;
    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) const;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) TypeSpec Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

// codegen: insert before includes
#include "azure/keyvault/secrets/keyvault_operations.hpp"
// codegen: end insert before includes

#include "azure/keyvault/secrets/models/secrets_models.hpp"
#include "azure/keyvault/secrets/secret_client_options.hpp"
#include "azure/keyvault/secrets/secret_client_paged_responses.hpp"

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  /**
   * @brief The key vault client performs cryptographic key operations and vault operations against
   * the Key Vault service.
   *
   */
  class SecretClient final {
  public:
    /**
     * @brief Constructs the SecretClient.
     * @param url The URL address where the client will send the requests to.
     * @param credential Credential to authenticate with the service.
     * @param options Optional parameters.
     *
     */
    explicit SecretClient(
        std::string const& url,
        std::shared_ptr<Core::Credentials::TokenCredential> const& credential,
        SecretClientOptions const& options = {});

    /**
     * @brief Gets the SecretClient URL endpoint.
     * @return The SecretClient's URL endpoint.
     *
     */
    std::string GetUrl() const;

    // codegen: insert before SecretClient::SetSecret
  private:
    // codegen: end insert before SecretClient::SetSecret

    /**
     * @brief The SET operation adds a secret to the Azure Key Vault. If the named secret already
     * exists, Azure Key Vault creates a new version of that secret. This operation requires the
     * secrets/set permission.
     * @param secretName The name of the secret. The value you provide may be copied globally for
     * the purpose of running the service. The value provided should not include personally
     * identifiable or sensitive information.
     * @param parameters The parameters for setting the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Response<Models::KeyVaultSecret> SetSecret(
        std::string const& secretName,
        Models::SecretSetParameters const& parameters,
        Core::Context const& context = {}) const;

    // codegen: insert after SecretClient::SetSecret
  public:
    /**
     * @brief The SET operation adds a secret to the Azure Key Vault. If the named secret already
     * exists, Azure Key Vault creates a new version of that secret. This operation requires the
     * secrets/set permission.
     * @param secretName The name of the secret. The value you provide may be copied globally for
     * the purpose of running the service. The value provided should not include personally
     * identifiable or sensitive information.
     * @param value The value of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Azure::Response<Models::KeyVaultSecret> SetSecret(
        std::string const& secretName,
        std::string const& value,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      Models::SecretSetParameters parameters;
      parameters.Value = value;
      return SetSecret(secretName, parameters, context);
    };

    /**
     * @brief The SET operation adds a secret to the Azure Key Vault. If the named secret already
     * exists, Azure Key Vault creates a new version of that secret. This operation requires the
     * secrets/set permission.
     * @param secretName The name of the secret. The value you provide may be copied globally for
     * the purpose of running the service. The value provided should not include personally
     * identifiable or sensitive information.
     * @param secret The secret to set.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Azure::Response<Models::KeyVaultSecret> SetSecret(
        std::string const& secretName,
        Models::KeyVaultSecret const& secret,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      Models::SecretSetParameters parameters;
      parameters.Value = secret.Value.Value();
      parameters.ContentType = secret.ContentType;
      parameters.Tags = secret.Tags;
      parameters.SecretAttributes = secret.Properties;
      return SetSecret(secretName, parameters, context);
    };
    // codegen: end insert after SecretClient::SetSecret

    // codegen: insert before SecretClient::DeleteSecret
  private:
    // codegen: end insert before SecretClient::DeleteSecret

    /**
     * @brief The DELETE operation applies to any secret stored in Azure Key Vault. DELETE cannot be
     * applied to an individual version of a secret. This operation requires the secrets/delete
     * permission.
     * @param secretName The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A Deleted Secret consisting of its previous id, attributes and its tags, as well as
     * information on when it will be purged.
     *
     */
    Response<Models::DeletedSecret> DeleteSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const;

    // codegen: insert after SecretClient::DeleteSecret
  public:
    /**
     * @brief The DELETE operation applies to any secret stored in Azure Key Vault. DELETE cannot be
     * applied to an individual version of a secret. This operation requires the secrets/delete
     * permission.
     * @param secretName The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A Deleted Secret consisting of its previous id, attributes and its tags, as well as
     * information on when it will be purged.
     *
     */
    DeleteSecretOperation StartDeleteSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const
    {
      return DeleteSecretOperation(
          secretName, std::make_shared<SecretClient>(*this), DeleteSecret(secretName, context));
    }

    // codegen: end insert after SecretClient::DeleteSecret

    // codegen: insert before SecretClient::UpdateSecretProperties
  private:
    // codegen: end insert before SecretClient::UpdateSecretProperties

    /**
     * @brief The UPDATE operation changes specified attributes of an existing stored secret.
     * Attributes that are not specified in the request are left unchanged. The value of a secret
     * itself cannot be changed. This operation requires the secrets/set permission.
     * @param secretName The name of the secret.
     * @param secretVersion The version of the secret.
     * @param parameters The parameters for update secret operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Response<Models::KeyVaultSecret> UpdateSecretProperties(
        std::string const& secretName,
        std::string const& secretVersion,
        Models::UpdateSecretPropertiesOptions const& parameters,
        Core::Context const& context = {}) const;

    //  codegen: insert after SecretClient::UpdateSecretProperties
  public:
    /**
     * @brief The UPDATE operation changes specified attributes of an existing stored secret.
     * Attributes that are not specified in the request are left unchanged. The value of a secret
     * itself cannot be changed. This operation requires the secrets/set permission.
     * @param secretName The name of the secret.
     * @param parameters The parameters for update secret operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Response<Models::KeyVaultSecret> UpdateSecretProperties(
        std::string const& secretName,
        Models::UpdateSecretPropertiesOptions const& parameters,
        Core::Context const& context = {}) const
    {
      return UpdateSecretProperties(
          secretName, parameters.SecretVersion.ValueOr(" "), parameters, context);
    }
    //  codegen: end insert after SecretClient::UpdateSecretProperties

    // codegen: replace SecretClient::GetSecret
  public:
    /**
     * @brief The GET operation is applicable to any secret stored in Azure Key Vault. This
     * operation requires the secrets/get permission.
     * @param secretName The name of the secret.
     * @param options Optional parameters.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Azure::Response<Models::KeyVaultSecret> GetSecret(
        std::string const& secretName,
        Models::GetSecretOptions const& options = Models::GetSecretOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;
    // codegen: end replace SecretClient::GetSecret

    /**
     * @brief The Get Secrets operation is applicable to the entire vault. However, only the base
     * secret identifier and its attributes are provided in the response. Individual secret versions
     * are not listed in the response. This operation requires the secrets/list permission.
     * @param options Optional parameters.
     * @param context The context for the operation can be used for request cancellation.
     * @return The secret list result.
     *
     */
    SecretPropertiesPagedResponse GetPropertiesOfSecrets(
        GetPropertiesOfSecretsOptions const& options = {},
        Core::Context const& context = {}) const;

    /**
     * @brief The full secret identifier and attributes are provided in the response. No values are
     * returned for the secrets. This operations requires the secrets/list permission.
     * @param secretName The name of the secret.
     * @param options Optional parameters.
     * @param context The context for the operation can be used for request cancellation.
     * @return The secret list result.
     *
     */
    SecretPropertiesVersionsPagedResponse GetPropertiesOfSecretsVersions(
        std::string const& secretName,
        GetPropertiesOfSecretVersionsOptions const& options = {},
        Core::Context const& context = {}) const;

    /**
     * @brief The Get Deleted Secrets operation returns the secrets that have been deleted for a
     * vault enabled for soft-delete. This operation requires the secrets/list permission.
     * @param options Optional parameters.
     * @param context The context for the operation can be used for request cancellation.
     * @return The deleted secret list result
     *
     */
    DeletedSecretPagedResponse GetDeletedSecrets(
        GetDeletedSecretsOptions const& options = {},
        Core::Context const& context = {}) const;

    /**
     * @brief The Get Deleted Secret operation returns the specified deleted secret along with its
     * attributes. This operation requires the secrets/get permission.
     * @param secretName The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A Deleted Secret consisting of its previous id, attributes and its tags, as well as
     * information on when it will be purged.
     *
     */
    Response<Models::DeletedSecret> GetDeletedSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const;

    /**
     * @brief The purge deleted secret operation removes the secret permanently, without the
     * possibility of recovery. This operation can only be enabled on a soft-delete enabled vault.
     * This operation requires the secrets/purge permission.
     * @param secretName The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return Operation result.
     *
     */
    Response<Models::PurgedSecret> PurgeDeletedSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const;

    // codegen: insert before SecretClient::RecoverDeletedSecret
  private:
    // codegen: end insert before SecretClient::RecoverDeletedSecret

    /**
     * @brief Recovers the deleted secret in the specified vault. This operation can only be
     * performed on a soft-delete enabled vault. This operation requires the secrets/recover
     * permission.
     * @param secretName The name of the deleted secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Response<Models::KeyVaultSecret> RecoverDeletedSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const;

    // codegen: insert after SecretClient::RecoverDeletedSecret
  public:
    /**
     * @brief Recovers the deleted secret in the specified vault. This operation can only be
     * performed on a soft-delete enabled vault. This operation requires the secrets/recover
     * permission.
     * @remark Recovers the deleted secret in the specified vault.
     * This operation can only be performed on a soft-delete enabled vault.
     * This operation requires the secrets/recover permission.
     *
     * @param name The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return RecoverDeletedSecretOperation
     */
    Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation StartRecoverDeletedSecret(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      return Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation(
          name, std::make_shared<SecretClient>(*this), RecoverDeletedSecret(name, context));
    }
    // codegen: end insert after SecretClient::RecoverDeletedSecret

    /**
     * @brief Requests that a backup of the specified secret be downloaded to the client. All
     * versions of the secret will be downloaded. This operation requires the secrets/backup
     * permission.
     * @param secretName The name of the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return The backup secret result, containing the backup blob.
     *
     */
    Response<Models::BackupSecretResult> BackupSecret(
        std::string const& secretName,
        Core::Context const& context = {}) const;

    // codegen: insert before SecretClient::RestoreSecretBackup
  private:
    // codegen: end insert before SecretClient::RestoreSecretBackup

    /**
     * @brief Restores a backed up secret, and all its versions, to a vault. This operation requires
     * the secrets/restore permission.
     * @param parameters The parameters to restore the secret.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Response<Models::KeyVaultSecret> RestoreSecretBackup(
        Models::SecretRestoreParameters const& parameters,
        Core::Context const& context = {}) const;

    // codegen: insert after SecretClient::RestoreSecretBackup
  public:
    /**
     * @brief Restores a backed up secret, and all its versions, to a vault. This operation requires
     * the secrets/restore permission.
     * @param backup The backup payload as encoded vector of bytes.
     * @param context The context for the operation can be used for request cancellation.
     * @return A secret consisting of a value, id and its attributes.
     *
     */
    Azure::Response<Models::KeyVaultSecret> RestoreSecretBackup(
        Models::BackupSecretResult const& backup,
        Azure::Core::Context const& context = Azure::Core::Context()) const
    {
      return RestoreSecretBackup(Models::SecretRestoreParameters{backup.Value.Value()}, context);
    }
    // codegen: end insert after SecretClient::RestoreSecretBackup

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_url;
    std::string m_apiVersion;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

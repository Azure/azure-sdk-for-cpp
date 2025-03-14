// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Keyvault Secrets Client definition.
 *
 */

#include "azure/keyvault/secrets/secret_client.hpp"

#include "azure/keyvault/secrets/generated.hpp"
#include "azure/keyvault/secrets/keyvault_operations.hpp"
#include "private/keyvault_protocol.hpp"
#include "private/package_version.hpp"
#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/keyvault/shared/keyvault_challenge_based_auth.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

#include <algorithm>
#include <string>
using namespace Azure::Core::Http;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Security::KeyVault::Secrets::_detail;

SecretClient::SecretClient(
    std::string const& vaultUrl,
    std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
    SecretClientOptions options)
{
  _detail::KeyVaultClientOptions generatedOptions;
  generatedOptions.ApiVersion = options.ApiVersion;
  generatedOptions.Log = options.Log;
  generatedOptions.Retry = options.Retry;
  generatedOptions.Transport = options.Transport;
  generatedOptions.Telemetry = options.Telemetry;
  generatedOptions.PerOperationPolicies = std::move(options.PerOperationPolicies);
  generatedOptions.PerRetryPolicies = std::move(options.PerRetryPolicies);
  m_client = std::make_shared<_detail::KeyVaultClient>(
      _detail::KeyVaultClient(vaultUrl, credential, generatedOptions));
}

Azure::Response<KeyVaultSecret> SecretClient::GetSecret(
    std::string const& name,
    GetSecretOptions const& options,
    Azure::Core::Context const& context) const
{
  auto secret = m_client->GetSecret(name, options.Version.empty() ? "/" : options.Version, context);
  KeyVaultSecret secretResult(secret.Value);
  secretResult.Properties.VaultUrl = m_vaultUrl.GetAbsoluteUrl();
  return Azure::Response<KeyVaultSecret>(std::move(secretResult), std::move(secret.RawResponse));
}

Azure::Response<DeletedSecret> SecretClient::GetDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->GetDeletedSecret(name, context);
  DeletedSecret secretResult(response.Value);
  secretResult.Properties.VaultUrl = m_vaultUrl.GetAbsoluteUrl();
  return Azure::Response<DeletedSecret>(std::move(secretResult), std::move(response.RawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::SetSecret(
    std::string const& name,
    std::string const& value,
    Azure::Core::Context const& context) const
{
  _detail::Models::SecretSetParameters secretParameters;
  secretParameters.Value = value;
  auto response = m_client->SetSecret(name, secretParameters, context);
  KeyVaultSecret secretResult(response.Value);
  return Azure::Response<KeyVaultSecret>(std::move(secretResult), std::move(response.RawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::SetSecret(
    std::string const& name,
    KeyVaultSecret const& secret,
    Azure::Core::Context const& context) const
{
  _detail::Models::SecretSetParameters secretParameters = secret.ToSetSecretParameters();
  auto response = m_client->SetSecret(name, secretParameters, context);
  KeyVaultSecret secretResult(response.Value);
  return Azure::Response<KeyVaultSecret>(std::move(secretResult), std::move(response.RawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::UpdateSecretProperties(
    SecretProperties const& properties,
    Azure::Core::Context const& context) const
{
  _detail::Models::SecretUpdateParameters secretParameters = properties.ToSecretUpdateParameters();
  auto response
      = m_client->UpdateSecret(properties.Name, properties.Version, secretParameters, context);
  KeyVaultSecret secretResult(response.Value);
  return Azure::Response<KeyVaultSecret>(std::move(secretResult), std::move(response.RawResponse));
}

Azure::Response<BackupSecretResult> SecretClient::BackupSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->BackupSecret(name, context);
  BackupSecretResult backupResult;
  backupResult.Secret = std::move(response.Value.Value.Value());
  return Azure::Response<BackupSecretResult>(
      std::move(backupResult), std::move(response.RawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::RestoreSecretBackup(
    BackupSecretResult const& backup,
    Azure::Core::Context const& context) const
{
  _detail::Models::SecretRestoreParameters restoreParameters;
  restoreParameters.SecretBundleBackup = backup.Secret;
  auto response = m_client->RestoreSecret(restoreParameters, context);
  KeyVaultSecret secretResult(response.Value);
  return Azure::Response<KeyVaultSecret>(std::move(secretResult), std::move(response.RawResponse));
}

Azure::Response<PurgedSecret> SecretClient::PurgeDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->PurgeDeletedSecret(name, context);
  PurgedSecret purgedResult;
  return Azure::Response<PurgedSecret>(std::move(purgedResult), std::move(response.RawResponse));
}

Azure::Security::KeyVault::Secrets::DeleteSecretOperation SecretClient::StartDeleteSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->DeleteSecret(name, context);
  DeletedSecret value(response.Value);
  auto responseT
      = Azure::Response<DeletedSecret>(std::move(value), std::move(response.RawResponse));

  return DeleteSecretOperation(std::make_shared<SecretClient>(*this), std::move(responseT));
}

Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation SecretClient::
    StartRecoverDeletedSecret(std::string const& name, Azure::Core::Context const& context) const
{
  auto response = m_client->RecoverDeletedSecret(name, context);
  KeyVaultSecret value(response.Value);
  value.Name = name;
  value.Properties.Name = name;
  auto responseT = Azure::Response<SecretProperties>(
      std::move(value.Properties), std::move(response.RawResponse));
  return RecoverDeletedSecretOperation(std::make_shared<SecretClient>(*this), std::move(responseT));
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecrets(
    GetPropertiesOfSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::KeyVaultClientGetSecretsOptions generatedOptions;
  if (options.NextPageToken.HasValue())
  {
    generatedOptions.NextPageToken = options.NextPageToken.Value();
  }
  auto response = m_client->GetSecrets(generatedOptions, context);
  SecretPropertiesPagedResponse secretPropertiesPagedResponse(
      response, std::move(response.RawResponse), std::make_unique<SecretClient>(*this));

  return secretPropertiesPagedResponse;
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecretsVersions(
    std::string const& name,
    GetPropertiesOfSecretVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::KeyVaultClientGetSecretVersionsOptions generatedOptions;
  if (options.NextPageToken.HasValue())
  {
    generatedOptions.NextPageToken = options.NextPageToken.Value();
  }
  auto response = m_client->GetSecretVersions(name, generatedOptions, context);
  SecretPropertiesPagedResponse secretPropertiesPagedResponse(
      response, std::move(response.RawResponse), std::make_unique<SecretClient>(*this), name);

  return secretPropertiesPagedResponse;
}

DeletedSecretPagedResponse SecretClient::GetDeletedSecrets(
    GetDeletedSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::KeyVaultClientGetDeletedSecretsOptions generatedOptions;
  if (options.NextPageToken.HasValue())
  {
    generatedOptions.NextPageToken = options.NextPageToken.Value();
  }
  _detail::GetDeletedSecretsPagedResponse response
      = m_client->GetDeletedSecrets(generatedOptions, context);
  DeletedSecretPagedResponse deletedSecretPagedResponse(
      response, std::move(response.RawResponse), std::make_unique<SecretClient>(*this));

  return deletedSecretPagedResponse;
}

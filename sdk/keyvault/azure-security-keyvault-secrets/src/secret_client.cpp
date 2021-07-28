// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keyvault Secrets Client definition.
 *
 */

#include "azure/keyvault/secrets/secret_client.hpp"

#include "private/keyvault_protocol.hpp"
#include "private/package_version.hpp"
#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
constexpr static const char TelemetryName[] = "keyvault-secrets";
}

const ServiceVersion ServiceVersion::V7_2("7.2");

SecretClient::SecretClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    SecretClientOptions options)
{
  auto apiVersion = options.Version.ToString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_protocolClient = std::make_shared<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient>(
      Azure::Core::Url(vaultUrl),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, TelemetryName, apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultSecret> SecretClient::GetSecret(
    std::string const& name,
    GetSecretOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(name, rawResponse);
      },
      {_detail::SecretPath, name, options.Version});
}

Azure::Response<KeyVaultDeletedSecret> SecretClient::GetDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultDeletedSecret>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
            name, rawResponse);
      },
      {_detail::DeletedSecretPath, name});
}

Azure::Response<KeyVaultSecret> SecretClient::SetSecret(
    std::string const& name,
    std::string const& value,
    Azure::Core::Context const& context) const
{
  KeyVaultSecret setParameters(name, value);
  return SetSecret(name, setParameters, context);
}

Azure::Response<KeyVaultSecret> SecretClient::SetSecret(
    std::string const& name,
    KeyVaultSecret const& secret,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Put,
      [&secret]() { return _detail::KeyVaultSecretSerializer::KeyVaultSecretSerialize(secret); },
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(name, rawResponse);
      },
      {_detail::SecretPath, name});
}

Azure::Response<KeyVaultSecret> SecretClient::UpdateSecretProperties(
    std::string const& name,
    UpdateSecretPropertiesOptions const& options,
    KeyvaultSecretProperties const& properties,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Patch,
      [&properties]() {
        return _detail::KeyVaultSecretPropertiesSerializer::KeyVaultSecretPropertiesSerialize(
            properties);
      },
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(name, rawResponse);
      },
      {_detail::SecretPath, name, options.Version});
}

Azure::Response<KeyVaultSecret> SecretClient::UpdateSecretProperties(
    std::string const& name,
    std::string const& version,
    KeyvaultSecretProperties const& properties,
    Azure::Core::Context const& context) const
{
  UpdateSecretPropertiesOptions options;
  options.Version = version;

  return UpdateSecretProperties(name, options, properties, context);
}

Azure::Response<BackupSecretData> SecretClient::BackupSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<BackupSecretData>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(
            rawResponse);
      },
      {_detail::SecretPath, name, _detail::BackupSecretPath});
}

Azure::Response<KeyVaultSecret> SecretClient::RestoreSecretBackup(
    BackupSecretData const& backup,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&backup]() {
        return _detail::KeyvaultRestoreSecretSerializer::KeyvaultRestoreSecretSerialize(backup);
      },
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(rawResponse);
      },
      {_detail::SecretPath, _detail::RestoreSecretPath});
}

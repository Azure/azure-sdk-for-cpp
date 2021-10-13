// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keyvault Secrets Client definition.
 *
 */

#include "azure/keyvault/secrets/secret_client.hpp"
#include "azure/keyvault/secrets/keyvault_operations.hpp"
#include "private/keyvault_protocol.hpp"
#include "private/package_version.hpp"
#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include <azure/keyvault/shared/keyvault_shared.hpp>

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <algorithm>
#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Security::KeyVault::Secrets::_detail;

namespace {
constexpr static const char TelemetryName[] = "keyvault-secrets";

struct RequestWithContinuationToken final
{
  std::vector<std::string> Path;
  std::unique_ptr<std::map<std::string, std::string>> Query;
};

static inline RequestWithContinuationToken BuildRequestFromContinuationToken(
    const Azure::Nullable<std::string>& NextPageToken,
    std::vector<std::string> defaultPath)
{
  RequestWithContinuationToken request;
  request.Path = std::move(defaultPath);
  request.Query = std::make_unique<std::map<std::string, std::string>>();
  if (NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(NextPageToken.Value());
    auto queryParameters = nextPageUrl.GetQueryParameters();
    request.Query->insert(queryParameters.begin(), queryParameters.end());
    request.Path.clear();
    request.Path.emplace_back(nextPageUrl.GetPath());
  }
  return request;
}

} // namespace

const ServiceVersion ServiceVersion::V7_2("7.2");

SecretClient::SecretClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    SecretClientOptions options)
{
  auto apiVersion = options.Version.ToString();
  Azure::Core::Url url(vaultUrl);

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{_internal::UrlScope::GetScopeFromUrl(url)}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_protocolClient = std::make_shared<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient>(
      std::move(url),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, TelemetryName, PackageVersion::ToString(), std::move(perRetrypolicies), {}));
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
        return _detail::SecretSerializer::Deserialize(name, rawResponse);
      },
      {_detail::SecretPath, name, options.Version});
}

Azure::Response<DeletedSecret> SecretClient::GetDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<DeletedSecret>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::DeletedSecretSerializer::Deserialize(name, rawResponse);
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
      [&secret]() { return _detail::SecretSerializer::Serialize(secret); },
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::SecretSerializer::Deserialize(name, rawResponse);
      },
      {_detail::SecretPath, name});
}

Azure::Response<KeyVaultSecret> SecretClient::UpdateSecretProperties(
    SecretProperties const& properties,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Patch,
      [&properties]() { return _detail::SecretPropertiesSerializer::Serialize(properties); },
      [&properties](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::SecretSerializer::Deserialize(properties.Name, rawResponse);
      },
      {_detail::SecretPath, properties.Name, properties.Version});
}

Azure::Response<BackupSecretResult> SecretClient::BackupSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<BackupSecretResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::BackupSecretSerializer::Deserialize(rawResponse);
      },
      {_detail::SecretPath, name, _detail::BackupSecretPath});
}

Azure::Response<KeyVaultSecret> SecretClient::RestoreSecretBackup(
    BackupSecretResult const& backup,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<KeyVaultSecret>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&backup]() { return _detail::RestoreSecretSerializer::Serialize(backup.Secret); },
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::SecretSerializer::Deserialize(rawResponse);
      },
      {_detail::SecretPath, _detail::RestoreSecretPath});
}

Azure::Response<PurgedSecret> SecretClient::PurgeDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_protocolClient->SendRequest<PurgedSecret>(
      context,
      Azure::Core::Http::HttpMethod::Delete,
      [](Azure::Core::Http::RawResponse const&) { return PurgedSecret(); },
      {_detail::DeletedSecretPath, name});
}

Azure::Security::KeyVault::Secrets::DeleteSecretOperation SecretClient::StartDeleteSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Secrets::DeleteSecretOperation(
      std::make_shared<SecretClient>(*this),
      m_protocolClient->SendRequest<DeletedSecret>(
          context,
          Azure::Core::Http::HttpMethod::Delete,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::DeletedSecretSerializer::Deserialize(name, rawResponse);
          },
          {_detail::SecretPath, name}));
}

Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation SecretClient::
    StartRecoverDeletedSecret(std::string const& name, Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation(
      std::make_shared<SecretClient>(*this),
      m_protocolClient->SendRequest<SecretProperties>(
          context,
          Azure::Core::Http::HttpMethod::Post,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            auto parsedResponse = _detail::SecretSerializer::Deserialize(name, rawResponse);
            return parsedResponse.Properties;
          },
          {_detail::DeletedSecretPath, name, _detail::RecoverDeletedSecretPath}));
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecrets(
    GetPropertiesOfSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request
      = BuildRequestFromContinuationToken(options.NextPageToken, {_detail::SecretPath});

  auto response = m_protocolClient->SendRequest<SecretPropertiesPagedResponse>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::SecretPropertiesPagedResultSerializer::Deserialize(rawResponse);
      },
      request.Path,
      request.Query);

  return SecretPropertiesPagedResponse(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<SecretClient>(*this));
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecretsVersions(
    std::string const& name,
    GetPropertiesOfSecretVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request = BuildRequestFromContinuationToken(
      options.NextPageToken, {_detail::SecretPath, name, _detail::VersionsName});

  auto response = m_protocolClient->SendRequest<SecretPropertiesPagedResponse>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::SecretPropertiesPagedResultSerializer::Deserialize(rawResponse);
      },
      request.Path,
      request.Query);

  return SecretPropertiesPagedResponse(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<SecretClient>(*this),
      name);
}

DeletedSecretPagedResponse SecretClient::GetDeletedSecrets(
    GetDeletedSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request
      = BuildRequestFromContinuationToken(options.NextPageToken, {_detail::DeletedSecretPath});

  auto response = m_protocolClient->SendRequest<DeletedSecretPagedResponse>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::DeletedSecretPagedResultSerializer::Deserialize(rawResponse);
      },
      request.Path,
      request.Query);

  return DeletedSecretPagedResponse(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<SecretClient>(*this));
}

std::string SecretClient::GetUrl() const { return m_protocolClient->GetUrl().GetAbsoluteUrl(); }

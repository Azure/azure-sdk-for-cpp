//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keyvault Secrets Client definition.
 *
 */

#include "azure/keyvault/secrets/secret_client.hpp"
#include "azure/keyvault/secrets/keyvault_operations.hpp"
#include "private/keyvault_protocol.hpp"
#include "private/keyvault_secrets_common_request.hpp"
#include "private/package_version.hpp"
#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

#include <algorithm>
#include <string>
using namespace Azure::Core::Http;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Security::KeyVault::Secrets::_detail;

std::unique_ptr<RawResponse> SecretClient::SendRequest(
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context) const
{
  return KeyVaultSecretsCommonRequest::SendRequest(*m_pipeline, request, context);
}

Request SecretClient::CreateRequest(
    HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return KeyVaultSecretsCommonRequest::CreateRequest(
      m_vaultUrl, m_apiVersion, method, path, content);
}

Request SecretClient::ContinuationTokenRequest(
    std::vector<std::string> const& path,
    const Azure::Nullable<std::string>& NextPageToken) const
{
  if (NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(NextPageToken.Value());
    return Request(HttpMethod::Get, nextPageUrl);
  }
  return CreateRequest(HttpMethod::Get, path);
}

SecretClient::SecretClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    SecretClientOptions options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  auto apiVersion = options.ApiVersion;
  Azure::Core::Url url(vaultUrl);

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{_internal::UrlScope::GetScopeFromUrl(url)}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      KeyVaultServicePackageName,
      PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

Azure::Response<KeyVaultSecret> SecretClient::GetSecret(
    std::string const& name,
    GetSecretOptions const& options,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {_detail::SecretPath, name, options.Version});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultSecret>(std::move(value), std::move(rawResponse));
}

Azure::Response<DeletedSecret> SecretClient::GetDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {_detail::DeletedSecretPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::DeletedSecretSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<DeletedSecret>(std::move(value), std::move(rawResponse));
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
  auto payload = _detail::SecretSerializer::Serialize(secret);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(HttpMethod::Put, {_detail::SecretPath, name}, &payloadStream);
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultSecret>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::UpdateSecretProperties(
    SecretProperties const& properties,
    Azure::Core::Context const& context) const
{
  auto payload = _detail::SecretPropertiesSerializer::Serialize(properties);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Patch,
      {_detail::SecretPath, properties.Name, properties.Version},
      &payloadStream);
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretSerializer::Deserialize(properties.Name, *rawResponse);
  return Azure::Response<KeyVaultSecret>(std::move(value), std::move(rawResponse));
}

Azure::Response<BackupSecretResult> SecretClient::BackupSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::SecretPath, name, _detail::BackupSecretPath});
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::BackupSecretSerializer::Deserialize(*rawResponse);
  return Azure::Response<BackupSecretResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultSecret> SecretClient::RestoreSecretBackup(
    BackupSecretResult const& backup,
    Azure::Core::Context const& context) const
{
  auto payload = _detail::RestoreSecretSerializer::Serialize(backup.Secret);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post, {_detail::SecretPath, _detail::RestoreSecretPath}, &payloadStream);
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretSerializer::Deserialize(*rawResponse);
  return Azure::Response<KeyVaultSecret>(std::move(value), std::move(rawResponse));
}

Azure::Response<PurgedSecret> SecretClient::PurgeDeletedSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {_detail::DeletedSecretPath, name});
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  PurgedSecret value;
  return Azure::Response<PurgedSecret>(std::move(value), std::move(rawResponse));
}

Azure::Security::KeyVault::Secrets::DeleteSecretOperation SecretClient::StartDeleteSecret(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {_detail::SecretPath, name});
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::DeletedSecretSerializer::Deserialize(name, *rawResponse);
  auto responseT = Azure::Response<DeletedSecret>(std::move(value), std::move(rawResponse));
  return DeleteSecretOperation(std::make_shared<SecretClient>(*this), std::move(responseT));
}

Azure::Security::KeyVault::Secrets::RecoverDeletedSecretOperation SecretClient::
    StartRecoverDeletedSecret(std::string const& name, Azure::Core::Context const& context) const
{
  auto request = CreateRequest(
      HttpMethod::Post, {_detail::DeletedSecretPath, name, _detail::RecoverDeletedSecretPath});
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto parsedResponse = _detail::SecretSerializer::Deserialize(name, *rawResponse);

  auto value = parsedResponse.Properties;
  auto responseT = Azure::Response<SecretProperties>(std::move(value), std::move(rawResponse));
  return RecoverDeletedSecretOperation(std::make_shared<SecretClient>(*this), std::move(responseT));
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecrets(
    GetPropertiesOfSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({SecretPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretPropertiesPagedResultSerializer::Deserialize(*rawResponse);
  return SecretPropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<SecretClient>(*this));
}

SecretPropertiesPagedResponse SecretClient::GetPropertiesOfSecretsVersions(
    std::string const& name,
    GetPropertiesOfSecretVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest(
      {_detail::SecretPath, name, _detail::VersionsName}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::SecretPropertiesPagedResultSerializer::Deserialize(*rawResponse);
  return SecretPropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<SecretClient>(*this), name);
}

DeletedSecretPagedResponse SecretClient::GetDeletedSecrets(
    GetDeletedSecretsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({_detail::DeletedSecretPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::DeletedSecretPagedResultSerializer::Deserialize(*rawResponse);
  return DeletedSecretPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<SecretClient>(*this));
}

std::string SecretClient::GetUrl() const { return m_vaultUrl.GetAbsoluteUrl(); }

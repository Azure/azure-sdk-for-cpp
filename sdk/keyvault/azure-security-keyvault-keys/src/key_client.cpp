// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include "azure/keyvault/keys/key_client.hpp"
#include "private/key_backup.hpp"
#include "private/key_constants.hpp"
#include "private/key_request_parameters.hpp"
#include "private/key_serializers.hpp"
#include "private/keyvault_protocol.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
constexpr static const char KeyVaultServicePackageName[] = "keyvault-keys";

struct RequestWithContinuationToken final
{
  std::vector<std::string> Path;
  std::unique_ptr<std::map<std::string, std::string>> Query;
};

static inline RequestWithContinuationToken BuildRequestFromContinuationToken(
    Azure::Security::KeyVault::Keys::GetPropertiesOfKeysOptions const& options,
    std::vector<std::string>&& defaultPath)
{
  RequestWithContinuationToken request;
  request.Path = defaultPath;
  if (options.NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(options.NextPageToken.Value());
    request.Query
        = std::make_unique<std::map<std::string, std::string>>(nextPageUrl.GetQueryParameters());
    request.Path.clear();
    request.Path.emplace_back(nextPageUrl.GetPath());
  }
  return request;
}

static inline RequestWithContinuationToken BuildRequestFromContinuationToken(
    Azure::Security::KeyVault::Keys::GetPropertiesOfKeyVersionsOptions const& options,
    std::vector<std::string>&& defaultPath)
{
  RequestWithContinuationToken request;
  request.Path = defaultPath;
  if (options.NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(options.NextPageToken.Value());
    request.Query
        = std::make_unique<std::map<std::string, std::string>>(nextPageUrl.GetQueryParameters());
    request.Path.clear();
    request.Path.emplace_back(nextPageUrl.GetPath());
  }
  return request;
}

static inline RequestWithContinuationToken BuildRequestFromContinuationToken(
    Azure::Security::KeyVault::Keys::GetDeletedKeysOptions const& options,
    std::vector<std::string>&& defaultPath)
{
  RequestWithContinuationToken request;
  request.Path = defaultPath;
  if (options.NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(options.NextPageToken.Value());
    request.Query
        = std::make_unique<std::map<std::string, std::string>>(nextPageUrl.GetQueryParameters());
    request.Path.clear();
    request.Path.emplace_back(nextPageUrl.GetPath());
  }
  return request;
}
} // namespace

KeyClient::KeyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    KeyClientOptions options)
{
  auto apiVersion = options.Version.ToString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient>(
      Azure::Core::Url(vaultUrl),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, KeyVaultServicePackageName, apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, options.Version});
}

Azure::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    KeyVaultKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(keyType, options),
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = ecKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(ecKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = rsaKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(rsaKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = octKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(octKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

KeyPropertiesPageResult KeyClient::GetPropertiesOfKeys(
    GetPropertiesOfKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request = BuildRequestFromContinuationToken(options, {_detail::KeysPath});
  auto response = m_pipeline->SendRequest<KeyPropertiesPageResult>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesPageResultSerializer::KeyPropertiesPageResultDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);

  return KeyPropertiesPageResult(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<KeyClient>(*this));
}

KeyPropertiesPageResult KeyClient::GetPropertiesOfKeyVersions(
    std::string const& name,
    GetPropertiesOfKeyVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request
      = BuildRequestFromContinuationToken(options, {_detail::KeysPath, name, "versions"});
  auto response = m_pipeline->SendRequest<KeyPropertiesPageResult>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesPageResultSerializer::KeyPropertiesPageResultDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);

  return KeyPropertiesPageResult(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<KeyClient>(*this),
      name);
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation KeyClient::StartDeleteKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      std::make_shared<KeyClient>(*this),
      m_pipeline->SendRequest<Azure::Security::KeyVault::Keys::DeletedKey>(
          context,
          Azure::Core::Http::HttpMethod::Delete,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::DeletedKeySerializer::DeletedKeyDeserialize(name, rawResponse);
          },
          {_detail::KeysPath, name}));
}

Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation KeyClient::StartRecoverDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation(
      std::make_shared<KeyClient>(*this),
      m_pipeline->SendRequest<Azure::Security::KeyVault::Keys::KeyVaultKey>(
          context,
          Azure::Core::Http::HttpMethod::Post,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, rawResponse);
          },
          {_detail::DeletedKeysPath, name, "recover"}));
}

Azure::Response<DeletedKey> KeyClient::GetDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<DeletedKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::DeletedKeySerializer::DeletedKeyDeserialize(name, rawResponse);
      },
      {_detail::DeletedKeysPath, name});
}

DeletedKeyPageResult KeyClient::GetDeletedKeys(
    GetDeletedKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request = BuildRequestFromContinuationToken(options, {_detail::DeletedKeysPath});
  auto response = m_pipeline->SendRequest<DeletedKeyPageResult>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesPageResultSerializer::DeletedKeyPageResultDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);

  return DeletedKeyPageResult(
      std::move(response.Value),
      std::move(response.RawResponse),
      std::make_unique<KeyClient>(*this));
}

Azure::Response<PurgedKey> KeyClient::PurgeDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<PurgedKey>(
      context,
      Azure::Core::Http::HttpMethod::Delete,
      [](Azure::Core::Http::RawResponse const&) { return PurgedKey(); },
      {_detail::DeletedKeysPath, name});
}

Azure::Response<KeyVaultKey> KeyClient::UpdateKeyProperties(
    KeyProperties const& properties,
    Azure::Nullable<std::list<KeyOperation>> const& keyOperations,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Patch,
      _detail::KeyRequestParameters(properties, keyOperations),
      [&properties](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(properties.Name, rawResponse);
      },
      {_detail::KeysPath, properties.Name, properties.Version});
}

Azure::Response<Azure::Security::KeyVault::Keys::BackupKeyResult> KeyClient::BackupKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Use the internal model KeyBackup to parse from Json
  auto response = m_pipeline->SendRequest<_detail::KeyBackup>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyBackup::Deserialize(rawResponse);
      },
      {_detail::KeysPath, name, "backup"});

  // Convert the internal KeyBackup model to a raw vector<uint8_t>.
  return Azure::Response<Azure::Security::KeyVault::Keys::BackupKeyResult>(
      Azure::Security::KeyVault::Keys::BackupKeyResult{response.Value.Value},
      std::move(response.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::RestoreKeyBackup(
    std::vector<uint8_t> const& backup,
    Azure::Core::Context const& context) const
{
  _detail::KeyBackup backupModel;
  backupModel.Value = backup;
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      backupModel,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(rawResponse);
      },
      {_detail::KeysPath, "restore"});
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    std::string const& name,
    JsonWebKey const& keyMaterial,
    Azure::Core::Context const& context) const
{
  ImportKeyOptions const importKeyOptions(name, keyMaterial);
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Put,
      [&importKeyOptions]() {
        return _detail::ImportKeyOptionsSerializer::ImportKeyOptionsSerialize(importKeyOptions);
      },
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name});
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    ImportKeyOptions const& importKeyOptions,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Put,
      [&importKeyOptions]() {
        return _detail::ImportKeyOptionsSerializer::ImportKeyOptionsSerialize(importKeyOptions);
      },
      [&importKeyOptions](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
            importKeyOptions.Name(), rawResponse);
      },
      {_detail::KeysPath, importKeyOptions.Name()});
}

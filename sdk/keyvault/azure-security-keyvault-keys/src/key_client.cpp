// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <azure/keyvault/common/internal/single_page.hpp>

#include "azure/keyvault/keys/details/key_backup.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_request_parameters.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"
#include "azure/keyvault/keys/key_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
struct RequestWithContinuationToken
{
  std::vector<std::string> Path;
  std::unique_ptr<std::map<std::string, std::string>> Query;
};

static inline RequestWithContinuationToken BuildRequestFromContinuationToken(
    Azure::Security::KeyVault::_internal::GetSinglePageOptions const& options,
    std::vector<std::string>&& defaultPath)
{
  RequestWithContinuationToken request;
  request.Path = defaultPath;
  if (options.ContinuationToken)
  {
    // Using a continuation token requires to send the request to the continuation token url instead
    // of the default url which is used only for the first page.
    Azure::Core::Url nextPageUrl(options.ContinuationToken.Value());
    request.Query
        = std::make_unique<std::map<std::string, std::string>>(nextPageUrl.GetQueryParameters());
    request.Path.clear();
    request.Path.emplace_back(nextPageUrl.GetPath());
  }
  if (options.MaxResults)
  {
    if (request.Query == nullptr)
    {
      request.Query = std::make_unique<std::map<std::string, std::string>>();
    }
    request.Query->emplace("maxResults", std::to_string(options.MaxResults.Value()));
  }
  return request;
}
} // namespace

KeyClient::KeyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    KeyClientOptions options)
{
  auto apiVersion = options.GetVersionString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::_internal::KeyVaultPipeline>(
      Azure::Core::Url(vaultUrl),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
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

Azure::Response<KeyPropertiesSinglePage> KeyClient::GetPropertiesOfKeysSinglePage(
    GetPropertiesOfKeysSinglePageOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request = BuildRequestFromContinuationToken(options, {_detail::KeysPath});
  return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesSinglePageSerializer::KeyPropertiesSinglePageDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);
}

Azure::Response<KeyPropertiesSinglePage> KeyClient::GetPropertiesOfKeyVersionsSinglePage(
    std::string const& name,
    GetPropertiesOfKeyVersionsSinglePageOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request
      = BuildRequestFromContinuationToken(options, {_detail::KeysPath, name, "versions"});
  return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesSinglePageSerializer::KeyPropertiesSinglePageDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation KeyClient::StartDeleteKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      m_pipeline,
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
      m_pipeline,
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

Azure::Response<DeletedKeySinglePage> KeyClient::GetDeletedKeysSinglePage(
    GetDeletedKeysSinglePageOptions const& options,
    Azure::Core::Context const& context) const
{
  auto const request = BuildRequestFromContinuationToken(options, {_detail::DeletedKeysPath});
  return m_pipeline->SendRequest<DeletedKeySinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesSinglePageSerializer::DeletedKeySinglePageDeserialize(
            rawResponse);
      },
      request.Path,
      request.Query);
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

Azure::Response<std::vector<uint8_t>> KeyClient::BackupKey(
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
  return Azure::Response<std::vector<uint8_t>>(
      response.Value.Value, std::move(response.RawResponse));
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

#include "azure/keyvault/keys/key_client.hpp"
#include "private/cryptography_internal_access.hpp"
#include "private/key_backup.hpp"
#include "private/key_constants.hpp"
#include "private/key_request_parameters.hpp"
#include "private/key_serializers.hpp"
#include "private/keyvault_protocol.hpp"
#include "private/package_version.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

namespace {
constexpr static const char KeyVaultServicePackageName[] = "keyvault-keys";
constexpr static const char CreateValue[] = "create";
} // namespace

std::unique_ptr<RawResponse> KeyClient::SendRequest(
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultKeysCommonRequest::SendRequest(
      *m_pipeline, request, context);
}

Request KeyClient::CreateRequest(
    HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultKeysCommonRequest::CreateRequest(
      m_vaultUrl, m_apiVersion, method, path, content);
}

Request KeyClient::ContinuationTokenRequest(
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

KeyClient::KeyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    KeyClientOptions options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.Version)
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{_internal::UrlScope::GetScopeFromUrl(m_vaultUrl)}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, std::move(tokenContext)));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      KeyVaultServicePackageName,
      PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

Azure::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Get, {_detail::KeysPath, name, options.Version});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    KeyVaultKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  _detail::KeyRequestParameters const params(keyType, options);
  auto payload = params.Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::KeysPath, name, CreateValue}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  std::string const& keyName = ecKeyOptions.GetName();
  auto payload = _detail::KeyRequestParameters(ecKeyOptions).Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::KeysPath, keyName, CreateValue}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  std::string const& keyName = rsaKeyOptions.GetName();
  auto payload = _detail::KeyRequestParameters(rsaKeyOptions).Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::KeysPath, keyName, CreateValue}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  // Payload for the request.
  std::string const& keyName = octKeyOptions.GetName();
  auto payload = _detail::KeyRequestParameters(octKeyOptions).Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::KeysPath, keyName, CreateValue}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(keyName, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

KeyPropertiesPagedResponse KeyClient::GetPropertiesOfKeys(
    GetPropertiesOfKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({_detail::KeysPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyPropertiesPagedResultSerializer::KeyPropertiesPagedResultDeserialize(
      *rawResponse);
  return KeyPropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<KeyClient>(*this));
}

KeyPropertiesPagedResponse KeyClient::GetPropertiesOfKeyVersions(
    std::string const& name,
    GetPropertiesOfKeyVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request
      = ContinuationTokenRequest({_detail::KeysPath, name, "versions"}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyPropertiesPagedResultSerializer::KeyPropertiesPagedResultDeserialize(
      *rawResponse);
  return KeyPropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<KeyClient>(*this));
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation KeyClient::StartDeleteKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Delete, {_detail::KeysPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::DeletedKeySerializer::DeletedKeyDeserialize(name, *rawResponse);
  auto responseT = Azure::Response<DeletedKey>(std::move(value), std::move(rawResponse));
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      std::make_shared<KeyClient>(*this), std::move(responseT));
}

Azure::Response<ReleaseKeyResult> KeyClient::ReleaseKey(
    std::string const& name,
    KeyReleaseOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(
      HttpMethod::Post,
      {_detail::KeysPath, name, options.Version.ValueOr(""), _detail::ReleaseValue},
      &payloadStream);

  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsDeserialize(*rawResponse);
  return Azure::Response<ReleaseKeyResult>(value, std::move(rawResponse));
}

Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation KeyClient::StartRecoverDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Post, {_detail::DeletedKeysPath, name, "recover"});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, *rawResponse);
  auto responseT = Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
  return Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation(
      std::make_shared<KeyClient>(*this), std::move(responseT));
}

Azure::Response<DeletedKey> KeyClient::GetDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Get, {_detail::DeletedKeysPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::DeletedKeySerializer::DeletedKeyDeserialize(name, *rawResponse);
  return Azure::Response<DeletedKey>(std::move(value), std::move(rawResponse));
}

DeletedKeyPagedResponse KeyClient::GetDeletedKeys(
    GetDeletedKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({_detail::DeletedKeysPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value
      = _detail::KeyPropertiesPagedResultSerializer::DeletedKeyPagedResultDeserialize(*rawResponse);
  return DeletedKeyPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<KeyClient>(*this));
}

Azure::Response<PurgedKey> KeyClient::PurgeDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Delete, {_detail::DeletedKeysPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = PurgedKey();
  return Azure::Response<PurgedKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::UpdateKeyProperties(
    KeyProperties const& properties,
    Azure::Nullable<std::vector<KeyOperation>> const& keyOperations,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  _detail::KeyRequestParameters const params(properties, keyOperations);
  auto payload = params.Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(
      HttpMethod::Patch, {_detail::KeysPath, properties.Name, properties.Version}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value
      = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(properties.Name, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<BackupKeyResult> KeyClient::BackupKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Post, {_detail::KeysPath, name, "backup"});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  // the internal backupKey model provides the Deserialize implementation
  auto internalValue = _detail::KeyBackup::Deserialize(*rawResponse);
  auto value = BackupKeyResult{internalValue.Value};
  return Azure::Response<BackupKeyResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::RestoreKeyBackup(
    std::vector<uint8_t> const& backup,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  _detail::KeyBackup backupModel;
  backupModel.Value = backup;
  auto payload = backupModel.Serialize();
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(HttpMethod::Post, {_detail::KeysPath, "restore"}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(*rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    std::string const& name,
    JsonWebKey const& keyMaterial,
    Azure::Core::Context const& context) const
{
  ImportKeyOptions const importKeyOptions(name, keyMaterial);
  return ImportKey(importKeyOptions, context);
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    ImportKeyOptions const& importKeyOptions,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  auto payload = _detail::ImportKeyOptionsSerializer::ImportKeyOptionsSerialize(importKeyOptions);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(
      HttpMethod::Put, {_detail::KeysPath, importKeyOptions.Name()}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
      importKeyOptions.Name(), *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::RotateKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request
      = CreateRequest(HttpMethod::Post, {_detail::KeysPath, name, _detail::RotateActionsValue});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyRotationPolicy> KeyClient::GetKeyRotationPolicy(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Request with no payload
  auto request
      = CreateRequest(HttpMethod::Get, {_detail::KeysPath, name, _detail::RotationPolicyPath});
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(*rawResponse);
  return Azure::Response<KeyRotationPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyRotationPolicy> KeyClient::UpdateKeyRotationPolicy(
    std::string const& name,
    KeyRotationPolicy const& rotationPolicy,
    Azure::Core::Context const& context) const
{
  // Payload for the request
  auto payload = _detail::KeyRotationPolicySerializer::KeyRotationPolicySerialize(rotationPolicy);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(
      HttpMethod::Put, {_detail::KeysPath, name, _detail::RotationPolicyPath}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(*rawResponse);
  return Azure::Response<KeyRotationPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<GetRandomBytesResult> KeyClient::GetRandomBytes(
    GetRandomBytesOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = _detail::GetRandomBytesSerializer::GetRandomBytesOptionsSerialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  // Request and settings
  auto request = CreateRequest(HttpMethod::Post, {"/rng"}, &payloadStream);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto response = GetRandomBytesResult{
      _detail::GetRandomBytesSerializer::GetRandomBytesResponseDeserialize(*rawResponse)};
  return Azure::Response<GetRandomBytesResult>(std::move(response), std::move(rawResponse));
}

Cryptography::CryptographyClient KeyClient::GetCryptographyClient(
    std::string const& name,
    std::string const& version) const
{
  Azure::Core::Url vaultUrl(m_vaultUrl);
  vaultUrl.AppendPath(_detail::KeysPath);
  vaultUrl.AppendPath(name);
  if (!version.empty())
  {
    vaultUrl.AppendPath(version);
  }

  return Cryptography::_detail::CryptoClientInternalAccess::CreateCryptographyClient(
      vaultUrl, m_apiVersion, m_pipeline);
}

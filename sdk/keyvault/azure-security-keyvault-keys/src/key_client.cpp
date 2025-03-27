// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/key_client.hpp"

#include "private/cryptography_internal_access.hpp"
#include "private/key_backup.hpp"
#include "private/key_constants.hpp"
#include "private/key_request_parameters.hpp"
#include "private/key_serializers.hpp"
#include "private/keyvault_protocol.hpp"
#include "private/package_version.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/keyvault/shared/keyvault_challenge_based_auth.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "./generated/key_vault_client.hpp"

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

namespace {
constexpr const char CreateValue[] = "create";
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
      m_vaultUrl, m_apiVersion, std::move(method), path, content);
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
    std::shared_ptr<const Core::Credentials::TokenCredential> credential,
    KeyClientOptions const& options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  _detail::KeyVaultClientOptions generatedClientOptions;
  static_cast<Core::_internal::ClientOptions&>(generatedClientOptions)
      = static_cast<const Core::_internal::ClientOptions&>(options);
  generatedClientOptions.ApiVersion = options.ApiVersion;
  m_client = std::make_shared<_detail::KeyVaultClient>(
      _detail::KeyVaultClient(vaultUrl, credential, generatedClientOptions));
}

Azure::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetKey(name, options.Version.empty() ? "/" : options.Version, context);
  KeyVaultKey keyResult(result.Value);
  keyResult.Properties.VaultUrl = m_vaultUrl.GetAbsoluteUrl();
  return Azure::Response<KeyVaultKey>(std::move(keyResult), std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    KeyVaultKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  Models::KeyCreateParameters keyCreateParameters = options.ToKeyCreateParameters();
  keyCreateParameters.Kty = Models::JsonWebKeyType(keyType.ToString());
  auto result = m_client->CreateKey(name, keyCreateParameters, context);
  KeyVaultKey keyResult(result.Value);
  return Azure::Response<KeyVaultKey>(keyResult, std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  Models::KeyCreateParameters keyCreateParameters
      = static_cast<CreateKeyOptions>(ecKeyOptions).ToKeyCreateParameters();
  keyCreateParameters.Kty = Models::JsonWebKeyType(ecKeyOptions.GetKeyType().ToString());
  if (ecKeyOptions.CurveName.HasValue())
  {
    keyCreateParameters.Curve
        = Models::JsonWebKeyCurveName(ecKeyOptions.CurveName.Value().ToString());
  }
  auto result = m_client->CreateKey(ecKeyOptions.GetName(), keyCreateParameters, context);
  KeyVaultKey keyResult(result.Value);
  return Azure::Response<KeyVaultKey>(keyResult, std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  Models::KeyCreateParameters keyCreateParameters
      = static_cast<CreateKeyOptions>(rsaKeyOptions).ToKeyCreateParameters();
  keyCreateParameters.Kty = Models::JsonWebKeyType(rsaKeyOptions.GetKeyType().ToString());
  if (rsaKeyOptions.KeySize.HasValue())
  {
    keyCreateParameters.KeySize = static_cast<int32_t>(rsaKeyOptions.KeySize.Value());
  }
  if (rsaKeyOptions.PublicExponent.HasValue())
  {
    keyCreateParameters.PublicExponent = static_cast<int32_t>(rsaKeyOptions.PublicExponent.Value());
  }
  auto result = m_client->CreateKey(rsaKeyOptions.GetName(), keyCreateParameters, context);
  KeyVaultKey keyResult(result.Value);
  return Azure::Response<KeyVaultKey>(keyResult, std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  Models::KeyCreateParameters keyCreateParameters
      = static_cast<CreateKeyOptions>(octKeyOptions).ToKeyCreateParameters();
  keyCreateParameters.Kty = Models::JsonWebKeyType(octKeyOptions.GetKeyType().ToString());
  if (octKeyOptions.KeySize.HasValue())
  {
    keyCreateParameters.KeySize = static_cast<int32_t>(octKeyOptions.KeySize.Value());
  }
  auto result = m_client->CreateKey(octKeyOptions.GetName(), keyCreateParameters, context);
  KeyVaultKey keyResult(result.Value);
  return Azure::Response<KeyVaultKey>(keyResult, std::move(result.RawResponse));
}

KeyPropertiesPagedResponse KeyClient::GetPropertiesOfKeys(
    GetPropertiesOfKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::KeyVaultClientGetKeysOptions getOptions;
  if (options.NextPageToken.HasValue())
  {
    getOptions.NextPageToken = options.NextPageToken.Value();
  }
  auto result = m_client->GetKeys(getOptions, context);
  return KeyPropertiesPagedResponse(
      std::move(result), std::move(result.RawResponse), std::make_unique<KeyClient>(*this));
}

KeyPropertiesPagedResponse KeyClient::GetPropertiesOfKeyVersions(
    std::string const& name,
    GetPropertiesOfKeyVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request
      = ContinuationTokenRequest({_detail::KeysPath, name, "versions"}, options.NextPageToken);

  // Send and parse response
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
  auto response = m_client->DeleteKey(name, context);
  // Request with no payload
  DeletedKey value(response.Value);
  auto responseT = Azure::Response<DeletedKey>(std::move(value), std::move(response.RawResponse));
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      std::make_shared<KeyClient>(*this), std::move(responseT));
}

Azure::Response<ReleaseKeyResult> KeyClient::ReleaseKey(
    std::string const& name,
    KeyReleaseOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::Models::KeyReleaseParameters keyReleaseParameters = options.ToKeyReleaseParameters();
  auto result = m_client->Release(name, options.Version.ValueOr(""),keyReleaseParameters, context);
  ReleaseKeyResult value{result.Value.Value.ValueOr("")};
  return Azure::Response<ReleaseKeyResult>(value, std::move(result.RawResponse));
}

Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation KeyClient::StartRecoverDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto result = m_client->RecoverDeletedKey(name, context);
  KeyVaultKey value(result.Value);
  auto responseT = Azure::Response<KeyVaultKey>(std::move(value), std::move(result.RawResponse));
  return Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation(
      std::make_shared<KeyClient>(*this), std::move(responseT));
}

Azure::Response<DeletedKey> KeyClient::GetDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->GetDeletedKey(name, context);
  DeletedKey value(response.Value);
  return Azure::Response<DeletedKey>(std::move(value), std::move(response.RawResponse));
}

DeletedKeyPagedResponse KeyClient::GetDeletedKeys(
    GetDeletedKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({_detail::DeletedKeysPath}, options.NextPageToken);

  // Send and parse response
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
  auto result = m_client->PurgeDeletedKey(name, context);
  auto value = PurgedKey();
  return Azure::Response<PurgedKey>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::UpdateKeyProperties(
    KeyProperties const& properties,
    Azure::Nullable<std::vector<KeyOperation>> const& keyOperations,
    Azure::Core::Context const& context) const
{
  auto result = m_client->UpdateKey(
      properties.Name,
      properties.Version,
      properties.ToKeyUpdateParameters(keyOperations),
      context);

  KeyVaultKey value(result.Value);
      
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<BackupKeyResult> KeyClient::BackupKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto response = m_client->BackupKey(name, context);
  auto internalValue = response.Value.Value.Value();
  auto value = BackupKeyResult{internalValue};
  return Azure::Response<BackupKeyResult>(std::move(value), std::move(response.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::RestoreKeyBackup(
    std::vector<uint8_t> const& backup,
    Azure::Core::Context const& context) const
{
  _detail::Models::KeyRestoreParameters restoreParameters{backup};
  auto response = m_client->RestoreKey(restoreParameters, context);
  KeyVaultKey value(response.Value);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(response.RawResponse));
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
  _detail::Models::KeyImportParameters keyImportParameters = importKeyOptions.ToKeyImportParameters();
  auto result = m_client->ImportKey(importKeyOptions.Name(),keyImportParameters, context);
  KeyVaultKey value(result.Value);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyVaultKey> KeyClient::RotateKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto result = m_client->RotateKey(name, context);
  KeyVaultKey value(result.Value);
  return Azure::Response<KeyVaultKey>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyRotationPolicy> KeyClient::GetKeyRotationPolicy(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetKeyRotationPolicy(name, context);
  KeyRotationPolicy value(result.Value);
  return Azure::Response<KeyRotationPolicy>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyRotationPolicy> KeyClient::UpdateKeyRotationPolicy(
    std::string const& name,
    KeyRotationPolicy const& rotationPolicy,
    Azure::Core::Context const& context) const
{
  _detail::Models::KeyRotationPolicy keyRotationPolicy = rotationPolicy.ToKeyRotationPolicy();
  auto result = m_client->UpdateKeyRotationPolicy(name, keyRotationPolicy, context);
  KeyRotationPolicy value(result.Value);
  return Azure::Response<KeyRotationPolicy>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<GetRandomBytesResult> KeyClient::GetRandomBytes(
    GetRandomBytesOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::Models::GetRandomBytesRequest getRandomBytesRequest;
  getRandomBytesRequest.Count = options.Count;
  auto result = m_client->GetRandomBytes(getRandomBytesRequest, context);
  auto value = GetRandomBytesResult{result.Value.Value};
  return Azure::Response<GetRandomBytesResult>(std::move(value), std::move(result.RawResponse));
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret serializers
 */

#include "private/secret_serializers.hpp"
#include "private/secret_constants.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

// Creates a new key based on a name and an HTTP raw response.
KeyVaultSecret SecretSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret;
  secret.Name = name;
  _detail::SecretSerializer::Deserialize(secret, rawResponse);
  return secret;
}

// Create from HTTP raw response only.
KeyVaultSecret SecretSerializer::Deserialize(Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret;
  _detail::SecretSerializer::Deserialize(secret, rawResponse);
  return secret;
}

// Updates a Key based on an HTTP raw response.
void SecretSerializer::Deserialize(
    KeyVaultSecret& secret,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  secret.Id = jsonParser[_detail::IdPropertyName];
  secret.Properties.Id = secret.Id;

  ParseIDUrl(secret.Properties, secret.Id);
  secret.Name = secret.Properties.Name;

  // Parse URL for the various attributes
  if (jsonParser.contains(_detail::AttributesPropertyName))
  {
    auto attributes = jsonParser[_detail::AttributesPropertyName];

    JsonOptional::SetIfExists(secret.Properties.Enabled, attributes, _detail::EnabledPropertyName);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.NotBefore,
        attributes,
        _detail::NbfPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.ExpiresOn,
        attributes,
        _detail::ExpPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.CreatedOn,
        attributes,
        _detail::CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Properties.UpdatedOn,
        attributes,
        _detail::UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<std::string>(
        secret.Properties.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
    JsonOptional::SetIfExists<int64_t>(
        secret.Properties.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
  }

  // "Tags"
  if (jsonParser.contains(_detail::TagsPropertyName))
  {
    auto const& tags = jsonParser[_detail::TagsPropertyName];
    {
      for (auto tag = tags.begin(); tag != tags.end(); ++tag)
      {
        secret.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
      }
    }
  }

  // managed
  if (jsonParser.contains(_detail::ManagedPropertyName))
  {
    secret.Properties.Managed = jsonParser[_detail::ManagedPropertyName].get<bool>();
  }

  // value
  if (jsonParser.contains(_detail::ValuePropertyName))
  {
    secret.Value = jsonParser[_detail::ValuePropertyName];
  }

  // key id
  JsonOptional::SetIfExists<std::string>(
      secret.Properties.KeyId, jsonParser, _detail::KeyIdPropertyName);

  // content type
  JsonOptional::SetIfExists<std::string>(
      secret.Properties.ContentType, jsonParser, _detail::ContentTypePropertyName);
}

DeletedSecret DeletedSecretSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  DeletedSecret deletedSecret(name);
  Deserialize(deletedSecret, rawResponse);
  return deletedSecret;
}

// Create deleted secret from HTTP raw response only.
DeletedSecret DeletedSecretSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  DeletedSecret deletedSecret;
  Deserialize(deletedSecret, rawResponse);
  return deletedSecret;
}

// Updates a deleted secret based on an HTTP raw response.
void DeletedSecretSerializer::Deserialize(
    DeletedSecret& secret,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  SecretSerializer::Deserialize(secret, rawResponse);

  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  secret.RecoveryId = jsonParser[_detail::RecoveryIdPropertyName];
  secret.ScheduledPurgeDate = PosixTimeConverter::PosixTimeToDateTime(
      jsonParser[_detail::ScheduledPurgeDatePropertyName]);
  secret.DeletedOn
      = PosixTimeConverter::PosixTimeToDateTime(jsonParser[_detail::DeletedDatePropertyName]);
}

// serializes a set secret parameters object
std::string SecretSerializer::Serialize(KeyVaultSecret const& parameters)
{
  json payload;

  JsonOptional::SetFromNullable(parameters.Value, payload, _detail::ValuePropertyName);

  JsonOptional::SetFromNullable(
      parameters.Properties.ContentType, payload, _detail::ContentTypePropertyName);

  json attributes;

  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.CreatedOn,
      attributes,
      _detail::CreatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(
      parameters.Properties.Enabled, attributes, _detail::EnabledPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.ExpiresOn,
      attributes,
      _detail::ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.NotBefore,
      attributes,
      _detail::NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(
      parameters.Properties.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
  JsonOptional::SetFromNullable(
      parameters.Properties.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.UpdatedOn,
      attributes,
      _detail::UpdatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  // optional tags
  attributes[TagsPropertyName] = json(parameters.Properties.Tags);

  payload[AttributesPropertyName] = attributes;

  return payload.dump();
}

std::string SecretPropertiesSerializer::Serialize(SecretProperties const& properties)
{
  json payload;

  // content type
  JsonOptional::SetFromNullable(properties.ContentType, payload, _detail::ContentTypePropertyName);

  // optional tags
  payload[TagsPropertyName] = json(properties.Tags);

  // attributes
  json attributes;

  JsonOptional::SetFromNullable(
      properties.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
  JsonOptional::SetFromNullable(
      properties.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
  JsonOptional::SetFromNullable(properties.Enabled, attributes, _detail::EnabledPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.NotBefore,
      attributes,
      _detail::NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.ExpiresOn,
      attributes,
      _detail::ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  payload[AttributesPropertyName] = attributes;

  return payload.dump();
}

BackupSecretResult BackupSecretSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  auto encodedResult = jsonParser[_detail::ValuePropertyName].get<std::string>();
  BackupSecretResult data;
  data.Secret = Base64Url::Base64UrlDecode(encodedResult);

  return data;
}

std::string RestoreSecretSerializer::Serialize(std::vector<uint8_t> const& backup)
{
  json payload;
  payload[_detail::ValuePropertyName] = Base64Url::Base64UrlEncode(backup);
  return payload.dump();
}

SecretPropertiesPagedResponse SecretPropertiesPagedResultSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  SecretPropertiesPagedResponse result;
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  JsonOptional::SetIfExists(result.NextPageToken, jsonParser, "nextLink");

  // Key properties
  auto secretsPropertiesJson = jsonParser["value"];

  for (auto const& secretProperties : secretsPropertiesJson)
  {
    SecretProperties item;
    item.Id = secretProperties[_detail::IdPropertyName].get<std::string>();
    _detail::SecretSerializer::ParseIDUrl(item, item.Id);
    // Parse URL for the various attributes
    if (secretProperties.contains(_detail::AttributesPropertyName))
    {
      auto attributes = secretProperties[_detail::AttributesPropertyName];

      JsonOptional::SetIfExists(item.Enabled, attributes, _detail::EnabledPropertyName);

      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.NotBefore,
          attributes,
          _detail::NbfPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.ExpiresOn,
          attributes,
          _detail::ExpPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.CreatedOn,
          attributes,
          _detail::CreatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.UpdatedOn,
          attributes,
          _detail::UpdatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<std::string>(
          item.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
      JsonOptional::SetIfExists<int64_t>(
          item.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
    }

    // "Tags"
    if (secretProperties.contains(_detail::TagsPropertyName))
    {
      auto const& tags = secretProperties[_detail::TagsPropertyName];
      {
        for (auto tag = tags.begin(); tag != tags.end(); ++tag)
        {
          item.Tags.emplace(tag.key(), tag.value().get<std::string>());
        }
      }
    }

    // managed
    if (secretProperties.contains(_detail::ManagedPropertyName))
    {
      item.Managed = secretProperties[_detail::ManagedPropertyName].get<bool>();
    }

    // content type
    JsonOptional::SetIfExists<std::string>(
        item.ContentType, secretProperties, _detail::ContentTypePropertyName);
    result.Items.emplace_back(item);
  }

  return result;
}

DeletedSecretPagedResponse DeletedSecretPagedResultSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{

  DeletedSecretPagedResponse result;
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  auto string = jsonParser.dump();
  JsonOptional::SetIfExists(result.NextPageToken, jsonParser, "nextLink");

  // Key properties
  auto secretsPropertiesJson = jsonParser["value"];

  for (auto const& secretProperties : secretsPropertiesJson)
  {
    DeletedSecret item;
    item.Id = secretProperties[_detail::IdPropertyName].get<std::string>();
    _detail::SecretSerializer::ParseIDUrl(item.Properties, item.Id);
    item.Name = item.Properties.Name;
    // Parse URL for the various attributes
    if (secretProperties.contains(_detail::AttributesPropertyName))
    {
      auto attributes = secretProperties[_detail::AttributesPropertyName];

      JsonOptional::SetIfExists(item.Properties.Enabled, attributes, _detail::EnabledPropertyName);

      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.Properties.NotBefore,
          attributes,
          _detail::NbfPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.Properties.ExpiresOn,
          attributes,
          _detail::ExpPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.Properties.CreatedOn,
          attributes,
          _detail::CreatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          item.Properties.UpdatedOn,
          attributes,
          _detail::UpdatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<std::string>(
          item.Properties.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
      JsonOptional::SetIfExists<int64_t>(
          item.Properties.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
    }

    // "Tags"
    if (secretProperties.contains(_detail::TagsPropertyName))
    {
      auto const& tags = secretProperties[_detail::TagsPropertyName];
      {
        for (auto tag = tags.begin(); tag != tags.end(); ++tag)
        {
          item.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
        }
      }
    }

    // managed
    if (secretProperties.contains(_detail::ManagedPropertyName))
    {
      item.Properties.Managed = secretProperties[_detail::ManagedPropertyName].get<bool>();
    }

    // content type
    JsonOptional::SetIfExists<std::string>(
        item.Properties.ContentType, secretProperties, _detail::ContentTypePropertyName);

    item.RecoveryId = secretProperties[_detail::RecoveryIdPropertyName];
    item.ScheduledPurgeDate = PosixTimeConverter::PosixTimeToDateTime(
        secretProperties[_detail::ScheduledPurgeDatePropertyName]);
    item.DeletedOn = PosixTimeConverter::PosixTimeToDateTime(
        secretProperties[_detail::DeletedDatePropertyName]);

    result.Items.emplace_back(item);
  }

  return result;
}
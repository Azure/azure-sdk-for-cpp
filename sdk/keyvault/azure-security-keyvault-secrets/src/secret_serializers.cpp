// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret serializers
 */

#include "private/secret_serializers.hpp"
#include "private/secret_constants.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

// Creates a new key based on a name and an HTTP raw response.
KeyVaultSecret KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret;
  secret.Name = name;
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, rawResponse);
  return secret;
}

// Create from HTTP raw response only.
KeyVaultSecret KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret secret;
  _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, rawResponse);
  return secret;
}

// Updates a Key based on an HTTP raw response.
void KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
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

KeyVaultDeletedSecret KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultDeletedSecret deletedSecret(name);
  KeyVaultDeletedSecretDeserialize(deletedSecret, rawResponse);
  return deletedSecret;
}

// Create deleted secret from HTTP raw response only.
KeyVaultDeletedSecret KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultDeletedSecret deletedSecret;
  KeyVaultDeletedSecretDeserialize(deletedSecret, rawResponse);
  return deletedSecret;
}

// Updates a deleted secret based on an HTTP raw response.
void KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
    KeyVaultDeletedSecret& secret,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecretSerializer::KeyVaultSecretDeserialize(secret, rawResponse);

  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  secret.RecoveryId = jsonParser[_detail::RecoveryIdPropertyName];
  secret.ScheduledPurgeDate = PosixTimeConverter::PosixTimeToDateTime(
      jsonParser[_detail::ScheduledPurgeDatePropertyName]);
  secret.DeletedDate
      = PosixTimeConverter::PosixTimeToDateTime(jsonParser[_detail::DeletedDatePropertyName]);
}

// serializes a set secret parameters object
std::string KeyVaultSecretSerializer::KeyVaultSecretSerialize(KeyVaultSecret const& parameters)
{
  json payload;
  using namespace Azure::Security::KeyVault::Secrets::_detail;

  // value is required
  payload[ValuePropertyName] = parameters.Value;

  // all else is optional
  JsonOptional::SetFromNullable(
      parameters.Properties.ContentType, payload, ContentTypePropertyName);

  json attributes;

  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.CreatedOn,
      attributes,
      CreatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(parameters.Properties.Enabled, attributes, EnabledPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.ExpiresOn,
      attributes,
      ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.NotBefore,
      attributes,
      NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(
      parameters.Properties.RecoverableDays, attributes, RecoverableDaysPropertyName);
  JsonOptional::SetFromNullable(
      parameters.Properties.RecoveryLevel, attributes, RecoveryLevelPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      parameters.Properties.UpdatedOn,
      attributes,
      UpdatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  // optional tags
  attributes[TagsPropertyName] = json(parameters.Properties.Tags);

  payload[AttributesPropertyName] = attributes;

  return payload.dump();
}

std::string KeyVaultSecretPropertiesSerializer::KeyVaultSecretPropertiesSerialize(
    KeyvaultSecretProperties const& properties)
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

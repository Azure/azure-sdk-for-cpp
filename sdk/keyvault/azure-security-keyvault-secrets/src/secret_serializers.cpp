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
  KeyVaultSecret secret(name);
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

  secret.Value = jsonParser[_detail::ValuePropertyName];
  secret.Id = jsonParser[_detail::IdPropertyName];

  // Parse URL for the various attributes
  if (jsonParser.contains(_detail::AttributesPropertyName))
  {
    auto attributes = jsonParser[_detail::AttributesPropertyName];

    JsonOptional::SetIfExists(secret.Attributes.Enabled, attributes, _detail::EnabledPropertyName);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Attributes.NotBefore,
        attributes,
        _detail::NbfPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Attributes.ExpiresOn,
        attributes,
        _detail::ExpPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Attributes.CreatedOn,
        attributes,
        _detail::CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        secret.Attributes.UpdatedOn,
        attributes,
        _detail::UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<std::string>(
        secret.Attributes.RecoveryLevel, attributes, _detail::RecoveryLevelPropertyName);
    JsonOptional::SetIfExists<int64_t>(
        secret.Attributes.RecoverableDays, attributes, _detail::RecoverableDaysPropertyName);
  }

  // "Tags"
  if (jsonParser.contains(_detail::TagsPropertyName))
  {
    auto const& tags = jsonParser[_detail::TagsPropertyName];
    {
      for (auto tag = tags.begin(); tag != tags.end(); ++tag)
      {
        secret.Tags.emplace(tag.key(), tag.value().get<std::string>());
      }
    }
  }

  // managed
  if (jsonParser.contains(_detail::ManagedPropertyName))
  {
    secret.Managed = jsonParser[_detail::ManagedPropertyName].get<bool>();
  }

  // key id
  JsonOptional::SetIfExists<std::string>(
      secret.KeyId, jsonParser, _detail::KeyIdPropertyName);

  // key id
  JsonOptional::SetIfExists<std::string>(secret.ContentType, jsonParser, _detail::ContentTypePropertyName);
}

// serializes a set secret parameters object
std::string KeyvaultSecretSetParametersSerializer::KeyvaultSecretSetParametersSerialize(
    KeyVaultSecretSetParameters const& parameters)
{
  Azure::Core::Json::_internal::json payload;
  using namespace Azure::Security::KeyVault::Secrets::_detail;
  
  // value is required
  payload[ValuePropertyName] = parameters.Value;

  // all else is optional
  JsonOptional::SetFromNullable(parameters.ContentType, payload, ContentTypePropertyName);

  if (parameters.Attributes.HasValue())
  { // optional attributes
    Azure::Core::Json::_internal::json attributes;

    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        parameters.Attributes.Value().CreatedOn,
        attributes,
        CreatedPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);
    JsonOptional::SetFromNullable(
        parameters.Attributes.Value().Enabled, attributes, EnabledPropertyName);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        parameters.Attributes.Value().ExpiresOn,
        attributes,
        ExpPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        parameters.Attributes.Value().NotBefore,
        attributes,
        NbfPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);
    JsonOptional::SetFromNullable(
        parameters.Attributes.Value().RecoverableDays, attributes, RecoverableDaysPropertyName);
    JsonOptional::SetFromNullable(
        parameters.Attributes.Value().RecoveryLevel, attributes, RecoveryLevelPropertyName);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        parameters.Attributes.Value().UpdatedOn,
        attributes,
        UpdatedPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);

    payload[AttributesPropertyName] = attributes;
  }

  if (parameters.Tags.HasValue())
  { // optional tags
    Azure::Core::Json::_internal::json tags;

    for (auto iterator  : parameters.Tags.Value())
    {
      tags[iterator.first] = iterator.second;
    }

    payload[TagsPropertyName] = tags;

  }

  return payload.dump();
}


// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include "azure/keyvault/keys/details/key_serializers.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"
#include "private/key_constants.hpp"

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Security::KeyVault::_internal::UnixTimeConverter;

KeyVaultKey _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultKey key(name);
  _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(key, rawResponse);
  return key;
}

KeyVaultKey _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultKey key;
  _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(key, rawResponse);
  return key;
}

void _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
    KeyVaultKey& key,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(key, jsonParser);
}

void _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(
    KeyVaultKey& key,
    Azure::Core::Json::_internal::json const& jsonParser)
{
  // Deserialize jwk
  _detail::JsonWebKeySerializer::JsonWebDeserialize(key.Key, jsonParser);

  // Parse URL for the vaultUri, keyVersion
  _detail::KeyVaultKeySerializer::ParseKeyUrl(key.Properties, key.Key.Id);

  // "Attributes"
  if (jsonParser.contains(_detail::AttributesPropertyName))
  {
    auto attributes = jsonParser[_detail::AttributesPropertyName];

    JsonOptional::SetIfExists(key.Properties.Enabled, attributes, _detail::EnabledPropertyName);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        key.Properties.NotBefore,
        attributes,
        _detail::NbfPropertyName,
        UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        key.Properties.ExpiresOn,
        attributes,
        _detail::ExpPropertyName,
        UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        key.Properties.CreatedOn,
        attributes,
        _detail::CreatedPropertyName,
        UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        key.Properties.UpdatedOn,
        attributes,
        _detail::UpdatedPropertyName,
        UnixTimeConverter::UnixTimeToDatetime);
  }

  // "Tags"
  if (jsonParser.contains(_detail::TagsPropertyName))
  {
    auto const& tags = jsonParser[_detail::TagsPropertyName];
    {
      for (auto tag = tags.begin(); tag != tags.end(); ++tag)
      {
        key.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
      }
    }
  }

  // managed
  if (jsonParser.contains(_detail::ManagedPropertyName))
  {
    key.Properties.Managed = jsonParser[_detail::ManagedPropertyName].get<bool>();
  }
}

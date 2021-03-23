// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/list_keys_single_page_result.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Security::KeyVault::Common::_internal::UnixTimeConverter;

ListKeysSinglePageResult _detail::ListKeysSinglePageResultDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  ListKeysSinglePageResult result;
  auto& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  JsonOptional::SetIfExists(result.ContinuationToken, jsonParser, "nextLink");

  // Keys
  auto keys = jsonParser["value"];
  for (auto const& key : keys)
  {
    KeyVaultKey keyVaultKey;
    keyVaultKey.Key.Id = key[_detail::KeyIdPropertyName].get<std::string>();
    _detail::ParseKeyUrl(keyVaultKey.Properties, keyVaultKey.Key.Id);
    // "Attributes"
    if (key.contains(_detail::AttributesPropertyName))
    {
      auto attributes = key[_detail::AttributesPropertyName];

      JsonOptional::SetIfExists(
          keyVaultKey.Properties.Enabled, attributes, _detail::EnabledPropertyName);
      JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
          keyVaultKey.Properties.NotBefore,
          attributes,
          _detail::NbfPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
          keyVaultKey.Properties.ExpiresOn,
          attributes,
          _detail::ExpPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
          keyVaultKey.Properties.CreatedOn,
          attributes,
          _detail::CreatedPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
          keyVaultKey.Properties.UpdatedOn,
          attributes,
          _detail::UpdatedPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
    }

    // "Tags"
    if (key.contains(_detail::TagsPropertyName))
    {
      auto const& tags = key[_detail::TagsPropertyName];
      {
        for (auto tag = tags.begin(); tag != tags.end(); ++tag)
        {
          keyVaultKey.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
        }
      }
    }

    // managed
    if (key.contains(_detail::ManagedPropertyName))
    {
      keyVaultKey.Properties.Managed = key[_detail::ManagedPropertyName].get<bool>();
    }

    result.Items.emplace_back(keyVaultKey);
  }

  return result;
}

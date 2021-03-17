// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_vault_key.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Security::KeyVault::Common::_internal::UnixTimeConverter;

namespace {
void ParseStringOperationsToKeyOperations(
    std::vector<KeyOperation>& keyOperations,
    std::vector<std::string> const& stringOperations)
{
  for (std::string const& operation : stringOperations)
  {
    keyOperations.emplace_back(KeyOperation(operation));
  }
}
} // namespace

KeyVaultKey _detail::KeyVaultKeyDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultKey key(name);
  _detail::KeyVaultKeyDeserialize(key, rawResponse);
  return key;
}

void _detail::KeyVaultKeyDeserialize(
    KeyVaultKey& key,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  // "Key"
  if (jsonParser.contains(_detail::KeyPropertyName))
  {
    auto const& jsonKey = jsonParser[_detail::KeyPropertyName];
    {
      // key_ops
      auto keyOperationVector
          = jsonKey[_detail::KeyOpsPropertyName].get<std::vector<std::string>>();
      std::vector<KeyOperation> keyOperations;
      ParseStringOperationsToKeyOperations(keyOperations, keyOperationVector);
      key.Key.SetKeyOperations(keyOperations);
    }
    key.Key.Id = jsonKey[_detail::KeyIdPropertyName].get<std::string>();
    key.Key.KeyType
        = _detail::KeyTypeFromString(jsonKey[_detail::KeyTypePropertyName].get<std::string>());

    JsonOptional::SetIfExists<std::string, KeyCurveName>(
        key.Key.CurveName, jsonKey, _detail::CurveNamePropertyName, [](std::string const& keyName) {
          return KeyCurveName(keyName);
        });
  }

  // "Attributes"
  if (jsonParser.contains(_detail::AttributesPropertyName))
  {
    auto attributes = jsonParser[_detail::AttributesPropertyName];

    JsonOptional::SetIfExists(key.Properties.Enabled, attributes, "enabled");
    JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
        key.Properties.NotBefore, attributes, "nbf", UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
        key.Properties.ExpiresOn, attributes, "exp", UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
        key.Properties.CreatedOn, attributes, "created", UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<uint64_t, Azure::DateTime>(
        key.Properties.UpdatedOn, attributes, "updated", UnixTimeConverter::UnixTimeToDatetime);
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_vault_key.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json.hpp>

using namespace Azure::Security::KeyVault::Keys;
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
  auto jsonParser = Azure::Core::_internal::Json::json::parse(body);

  // "Key"
  auto const& jsonKey = jsonParser[Details::KeyPropertyName];
  {
    // key_ops
    auto keyOperationVector = jsonKey[Details::KeyOpsPropertyName].get<std::vector<std::string>>();
    std::vector<KeyOperation> keyOperations;
    ParseStringOperationsToKeyOperations(keyOperations, keyOperationVector);
    key.Key.SetKeyOperations(keyOperations);
  }
  key.Key.Id = jsonKey[Details::KeyIdPropertyName].get<std::string>();
  key.Key.KeyType
      = _detail::KeyTypeFromString(jsonKey[Details::KeyTypePropertyName].get<std::string>());

  if (jsonKey.contains(_detail::CurveNamePropertyName))
  {
    key.Key.CurveName = KeyCurveName(jsonKey[Details::CurveNamePropertyName].get<std::string>());
  }

  // "Attributes"
  {
    auto attributes = jsonParser[Details::AttributesPropertyName];
    key.Properties.CreatedOn
        = UnixTimeConverter::UnixTimeToDatetime(attributes["created"].get<uint64_t>());
  }

  // "Tags"
  auto const& tags = jsonParser[Details::TagsPropertyName];
  {
    for (auto tag = tags.begin(); tag != tags.end(); ++tag)
    {
      key.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
    }
  }
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_vault_key.hpp"
#include "azure/keyvault/keys/key_constants.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json.hpp>

using namespace Azure::Security::KeyVault::Keys;
using Azure::Security::KeyVault::Common::Internal::UnixTimeConverter;

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

KeyVaultKey Details::KeyVaultKeyDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultKey key(name);
  Details::KeyVaultKeyDeserialize(key, rawResponse);
  return key;
}

void Details::KeyVaultKeyDeserialize(
    KeyVaultKey& key,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto body = rawResponse.GetBody();
  auto jsonParser = Azure::Core::Internal::Json::json::parse(body);

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
      = Details::KeyTypeFromString(jsonKey[Details::KeyTypePropertyName].get<std::string>());

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

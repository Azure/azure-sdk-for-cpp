// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_vault_key.hpp"

#include <nlohmann/json.hpp>

using namespace Azure::Security::KeyVault::Keys;

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
  auto body = rawResponse.GetBody();
  auto jsonParser = nlohmann::json::parse(body);

  KeyVaultKey key(name);

  {
    auto keyOperationVector = jsonParser["key"]["key_ops"].get<std::vector<std::string>>();
    std::vector<KeyOperation> keyOperations;
    ParseStringOperationsToKeyOperations(keyOperations, keyOperationVector);
    key.Key.SetKeyOperations(keyOperations);
  }

  key.Key.Id = jsonParser["key"]["kid"].get<std::string>();
  key.Key.KeyType = Details::KeyTypeFromString(jsonParser["key"]["kty"].get<std::string>());

  return key;
}

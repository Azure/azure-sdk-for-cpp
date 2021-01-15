// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_properties.hpp"

#include <azure/core/http/http.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* KeyPropertyName = "key";
  } // namespace Details

  struct KeyVaultKey
  {
    JsonWebKey Key;
    KeyProperties Properties;

    KeyVaultKey(std::string name) : Properties(std::move(name)) {}

    std::string const& Id() const { return Key.Id; }
    std::string const& Name() const { return Properties.Name; }
    KeyTypeEnum const& GetKeyType() const { return Key.KeyType; }
    std::vector<KeyOperation> const& KeyOperations() const { return Key.KeyOperations(); }
  };

  /***********************  Deserializer / Serializer ******************************/
  namespace Details {
    KeyVaultKey KeyVaultKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);
  } // namespace Details

}}}} // namespace Azure::Security::KeyVault::Keys

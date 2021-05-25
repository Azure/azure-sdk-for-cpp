// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Key.
 *
 */

#pragma once

#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_properties.hpp"

#include <azure/core/http/http.hpp>

#include <string>
#include <vector>
namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief A key resource and its properties.
   *
   */
  struct KeyVaultKey
  {
    /**
     * @brief Destructor.
     *
     */
    virtual ~KeyVaultKey() = default;

    /**
     * @brief The cryptographic key, the key type, and the operations you can perform using the key.
     *
     */
    JsonWebKey Key;

    /**
     * @brief The additional properties.
     *
     */
    KeyProperties Properties;

    /**
     * @brief Construct an empty Key.
     *
     */
    KeyVaultKey() = default;

    /**
     * @brief Construct a new Key Vault Key object.
     *
     * @param name The name of the key.
     */
    KeyVaultKey(std::string name) : Properties(std::move(name)) {}

    /**
     * @brief Get the Key identifier.
     *
     * @return The key ID.
     */
    std::string const& Id() const { return Key.Id; }

    /**
     * @brief Gets the name of the Key.
     *
     * @return The name of the key.
     */
    std::string const& Name() const { return Properties.Name; }

    /**
     * @brief Get the Key Type.
     *
     * @return The type of the key.
     */
    KeyVaultKeyType const& GetKeyType() const { return Key.KeyType; }

    /**
     * @brief Gets the operations you can perform using the key.
     *
     * @return A vector with the supported operations for the key.
     */
    std::vector<KeyOperation> const& KeyOperations() const { return Key.KeyOperations(); }
  };
}}}} // namespace Azure::Security::KeyVault::Keys

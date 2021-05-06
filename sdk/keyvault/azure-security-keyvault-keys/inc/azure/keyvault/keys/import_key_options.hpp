// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the properties to import a Key.
 *
 */

#pragma once

#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_properties.hpp"

#include <azure/core/http/http.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief A key resource and its properties.
   *
   */
  struct ImportKeyOptions
  {
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
     * @brief Get or Set a value indicating whether to import the key into a hardware security
     * module (HSM).
     *
     */
    Azure::Nullable<bool> HardwareProtected;

    /**
     * @brief Construct a new Key Vault ImportKeyOptions object.
     *
     * @param name The name of the key.
     */
    ImportKeyOptions(std::string name, JsonWebKey keyMaterial)
        : Key(keyMaterial), Properties(std::move(name))
    {
    }

    /**
     * @brief Gets the name of the Key.
     *
     * @return The name of the key.
     */
    std::string const& Name() const { return Properties.Name; }
  };
}}}} // namespace Azure::Security::KeyVault::Keys

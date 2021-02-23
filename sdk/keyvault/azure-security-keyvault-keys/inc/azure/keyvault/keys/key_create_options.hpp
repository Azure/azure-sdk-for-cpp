// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Key.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_operation.hpp"

#include <list>
#include <string>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Define the specific options for the #CreateKey operaion.
   *
   */
  struct CreateKeyOptions
  {
    /**
     * @brief Define the supported operations for the key.
     *
     */
    std::list<KeyOperation> KeyOperations;

    /**
     * @brief Indicates when the key will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Core::Nullable<Azure::Core::DateTime> NotBefore;

    /**
     * @brief Indicates when the key will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Core::Nullable<Azure::Core::DateTime> ExpiresOn;

    /**
     * @brief whether the key is enabled and useable for cryptographic operations.
     *
     */
    Azure::Core::Nullable<bool> Enabled;

    /**
     * @brief Specific metadata about the key.
     *
     */
    std::unordered_map<std::string, std::string> Tags;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

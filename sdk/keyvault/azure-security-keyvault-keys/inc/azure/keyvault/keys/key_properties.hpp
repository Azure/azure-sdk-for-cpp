// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Key properties.
 *
 */

#pragma once

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include <string>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The resource containing all the properties of the KeyVaultKey except JsonWebKey
   * properties.
   *
   */
  struct KeyProperties
  {
    /**
     * @brief The name of the key.
     *
     */
    std::string Name;

    /**
     * @brief The key identifier.
     *
     */
    std::string Id;

    /**
     * @brief The Key Vault base Url.
     *
     */
    std::string VaultUrl;

    /**
     * @brief The version of the key.
     *
     */
    std::string Version;

    /**
     * @brief Indicate whether the key's lifetime is managed by Key Vault. If this key is backing a
     * Key Vault certificate, the value will be true.
     *
     */
    bool Managed;

    /**
     * @brief Dictionary of tags with specific metadata about the key.
     *
     */
    std::unordered_map<std::string, std::string> Tags;

    /**
     * @brief Indicate whether the key is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Indicate when the key will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicate when the key will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief Indicate when the key was created.
     *
     */
    Azure::Nullable<Azure::DateTime> CreatedOn;

    /**
     * @brief Indicate when the key was updated.
     *
     */
    Azure::Nullable<Azure::DateTime> UpdatedOn;

    /**
     * @brief The number of days a key is retained before being deleted for a soft delete-enabled
     * Key Vault.
     *
     */
    Azure::Nullable<int> RecoverableDays;

    /**
     * @brief The recovery level currently in effect for keys in the Key Vault.
     *
     * @remark If Purgeable, the key can be permanently deleted by an authorized user; otherwise,
     * only the service can purge the keys at the end of the retention interval.
     *
     */
    std::string RecoveryLevel;

    /**
     * @brief Construct a new Key Properties object.
     *
     */
    KeyProperties() = default;

    /**
     * @brief Construct a new Key Properties object.
     *
     * @param name The name of the key.
     */
    KeyProperties(std::string name) : Name(std::move(name)) {}
  };

}}}} // namespace Azure::Security::KeyVault::Keys

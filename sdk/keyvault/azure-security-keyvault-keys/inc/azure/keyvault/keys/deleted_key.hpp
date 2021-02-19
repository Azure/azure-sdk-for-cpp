// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Represents a Key Vault key that has been deleted, allowing it to be recovered, if needed.
 *
 */

#pragma once

#include <azure/core/datetime.hpp>

#include "azure/keyvault/keys/key_constants.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Represents a Key Vault key that has been deleted, allowing it to be recovered, if
   * needed.
   *
   */
  struct DeletedKey : public KeyVaultKey
  {
    /**
     * @brief A recovery url that can be used to recover it.
     *
     */
    std::string RecoveryId;

    /**
     * @brief Construct an empty DeletedKey
     *
     */
    DeletedKey() = default;

    /**
     * @brief Construct a new Deleted Key object.
     *
     * @param name The name of the deleted key.
     */
    DeletedKey(std::string name) : KeyVaultKey(name) {}

    /**
     * @brief Indicate when the key was deleted.
     *
     */
    Azure::Core::DateTime DeletedDate;

    /**
     * @brief Indicate when the deleted key will be purged.
     *
     */
    Azure::Core::DateTime ScheduledPurgeDate;
  };

  /***********************  Deserializer / Serializer ******************************/
  namespace Details {
    DeletedKey DeletedKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);
  } // namespace Details

}}}} // namespace Azure::Security::KeyVault::Keys

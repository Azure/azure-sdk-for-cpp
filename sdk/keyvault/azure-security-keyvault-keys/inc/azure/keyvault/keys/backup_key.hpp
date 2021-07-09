// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the backupKey type.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <cstdint>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The JsonWebKey types.
   *
   */
  class BackupKey final {
  private:
    std::vector<uint8_t> m_value;

    BackupKey() = delete;

  public:
    /**
     * @brief Construct a new backupKey type object.
     *
     * @param backupKey The backup key array data.
     */
    explicit BackupKey(std::vector<uint8_t> backupKey) : m_value(std::move(backupKey)) {}

    /**
     * @brief Return the backup key.
     *
     */
    std::vector<uint8_t> const& GetValue() const { return m_value; }
  };

}}}} // namespace Azure::Security::KeyVault::Keys

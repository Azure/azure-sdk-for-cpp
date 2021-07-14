// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the BackupKey model type.
 *
 */

#pragma once

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The BackupKeyResult type containing the backup key of bytes.
   *
   */
  struct BackupKeyResult final
  {
    /**
     * @brief The backup key array data.
     *
     */
    std::vector<uint8_t> BackupKey;
  };

}}}} // namespace Azure::Security::KeyVault::Keys
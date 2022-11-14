//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault BackupSecretData definition
 */
#pragma once
#include <stdint.h>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  /**
   * @brief Represents a backed up secret.
   *
   */
  struct BackupSecretResult final
  {
    /**
     * @brief The backed up secret.
     *
     */
    std::vector<uint8_t> Secret;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

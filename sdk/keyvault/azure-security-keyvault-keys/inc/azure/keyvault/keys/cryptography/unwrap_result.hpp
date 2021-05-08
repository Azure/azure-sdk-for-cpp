// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about an unwrap operation.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/key_wrap_algorithm.hpp"

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Represents information about an unwrap operation.
   *
   */
  struct UnwrapResult final
  {
    /**
     * @brief Gets the key identifier of the #Key used to uwrap.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the unwrapped key.
     *
     */
    std::vector<uint8_t> Key;

    /**
     * @brief Gets the #KeyWrapAlgorithm used. This must be stored alongside the
     * #EncryptedKey as the same algorithm must be used to unwrap it.
     *
     */
    KeyWrapAlgorithm Algorithm;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

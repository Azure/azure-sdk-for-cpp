// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about a wrap operation.
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
   * @brief Represents information about a wrap operation.
   *
   */
  struct WrapResult
  {
    /**
     * @brief Gets the key identifier of the #KeyVaultKey used to encrypt. This must be stored
     * alongside the #Ciphertext as the same key must be used to decrypt it.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the wrapped key.
     *
     */
    std::vector<uint8_t> EncryptedKey;

    /**
     * @brief Gets the #KeyWrapAlgorithm used. This must be stored alongside the
     * #EncryptedKey as the same algorithm must be used to unwrap it.
     *
     */
    KeyWrapAlgorithm Algorithm;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

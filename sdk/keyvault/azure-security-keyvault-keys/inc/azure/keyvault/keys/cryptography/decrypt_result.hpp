// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about an encryption operation.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/encryption_algorithm.hpp"

#include <string>
#include <vector>
namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Represents information about an encryption operation.
   *
   */
  struct DecryptResult final
  {
    /**
     * @brief Gets the key identifier of the #KeyVaultKey used to encrypt. This must be stored
     * alongside the #Ciphertext as the same key must be used to decrypt it.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the ciphertext that is the result of the encryption.
     *
     */
    std::vector<uint8_t> Plaintext;

    /**
     * @brief Gets the #EncryptionAlgorithm used for encryption. This must be stored alongside the
     * #Ciphertext as the same algorithm must be used to decrypt it.
     *
     */
    EncryptionAlgorithm Algorithm;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

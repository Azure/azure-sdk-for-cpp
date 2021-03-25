// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about a decrypt operation.
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
   * @brief Represents information about a decrypt operation.
   *
   */
  struct DecryptResult
  {
    /**
     * @brief Gets the key identifier of the see #KeyVaultKey used to decrypt.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the decrypted data.
     *
     */
    std::vector<uint8_t> Plaintext;

    /**
     * @brief Gets the #EncryptionAlgorithm used for the decryption..
     *
     */
    EncryptionAlgorithm Algorithm;

    DecryptResult(EncryptionAlgorithm const& algorithm) : Algorithm(algorithm) {}
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

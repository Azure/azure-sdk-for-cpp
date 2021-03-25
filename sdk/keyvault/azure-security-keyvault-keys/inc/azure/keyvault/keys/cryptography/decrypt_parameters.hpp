// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define parameters for decrypting ciphertext.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/encryption_algorithm.hpp"

#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Define parameters for decrypting ciphertext.
   *
   */
  struct DecryptParameters
  {
    /**
     * @brief Gets or sets the <see cref="EncryptionAlgorithm"/>.
     */
    EncryptionAlgorithm Algorithm;

    /**
     * @brief Gets the ciphertext to decrypt.
     */
    std::vector<uint8_t> Ciphertext;

    /**
     * @brief Gets the initialization vector for decryption.
     */
    std::vector<uint8_t> Iv;

    /**
     * @brief Gets the authenticated tag resulting from encryption with a symmetric key using AES.
     */
    std::vector<uint8_t> AuthenticationTag;

    /**
     * @brief Gets additional data that is authenticated during decryption but not encrypted.
     */
    std::vector<uint8_t> AdditionalAuthenticatedData;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

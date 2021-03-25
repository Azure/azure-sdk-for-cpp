// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define parameters for encrypting plaintext.
 *
 */

#pragma once

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Define parameters for encrypting plaintext.
   *
   */
  struct EncryptParameters
  {
    /**
     * @brief Gets the <see cref="EncryptionAlgorithm"/>.
     */
    EncryptionAlgorithm Algorithm;

    /**
     * @brief Gets the plaintext to encrypt.
     */
    std::vector<uint8_t> Plaintext;

    /**
     * @brief Gets the initialization vector for encryption.
     */
    std::vector<uint8_t> Iv;

    /**
     * @brief Gets additional data that is authenticated during decryption but not encrypted.
     */
    std::vector<uint8_t> AdditionalAuthenticatedData;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

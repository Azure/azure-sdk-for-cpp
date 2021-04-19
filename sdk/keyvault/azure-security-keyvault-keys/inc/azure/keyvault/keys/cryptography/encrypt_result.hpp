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

  struct EncryptResult
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
    std::vector<uint8_t> Ciphertext;

    /**
     * @brief Gets the initialization vector for encryption.
     *
     */
    std::vector<uint8_t> Iv;

    /**
     * @brief Gets the authentication tag resulting from encryption with a symmetric key including
     * #EncryptionAlgorithm.A128Gcm, #EncryptionAlgorithm.A192Gcm, or #EncryptionAlgorithm.A256Gcm.
     *
     */
    std::vector<uint8_t> AuthenticationTag;

    /**
     * @brief Gets additional data that is authenticated during decryption but not encrypted.
     *
     */
    std::vector<uint8_t> AdditionalAuthenticatedData;

    /**
     * @brief Gets the #EncryptionAlgorithm used for encryption. This must be stored alongside the
     * #Ciphertext as the same algorithm must be used to decrypt it.
     *
     */
    EncryptionAlgorithm Algorithm;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

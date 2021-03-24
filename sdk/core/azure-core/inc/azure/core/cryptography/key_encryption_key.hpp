// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A key which is used to encrypt, or wrap, another key.
 */

#pragma once

#include <azure/core/context.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Cryptography {

  /**
   * @brief A key which is used to encrypt, or wrap, another key.
   *
   */
  struct IKeyEncryptionKey
  {
    /**
     * @brief The Id of the key used to perform cryptographic operations for the client.
     *
     */
    std::string KeyId;

    /**
     * @brief Encrypts the specified key using the specified algorithm.
     *
     * @param algorithm The key wrap algorithm used to encrypt the specified key.
     * @param key The key to be encrypted.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return The encrypted key bytes.
     */
    virtual std::vector<uint8_t> WrapKey(
        std::string const& algorithm,
        std::vector<uint8_t> const& key,
        Azure::Core::Context const& context) const = 0;

    /**
     * @brief Decrypts the specified encrypted key using the specified algorithm.
     *
     * @param algorithm The key wrap algorithm which was used to encrypt the specified encrypted
     * key.
     * @param encryptedKey The encrypted key to be decrypted.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return std::vector<uint8_t>
     */
    virtual std::vector<uint8_t> UnwrapKey(
        std::string const& algorithm,
        std::vector<uint8_t> const& encryptedKey,
        Azure::Core::Context const& context) const = 0;
  };
}}} // namespace Azure::Core::Cryptography

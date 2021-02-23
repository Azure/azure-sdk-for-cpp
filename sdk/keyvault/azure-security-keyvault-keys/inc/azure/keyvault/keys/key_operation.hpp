// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault KeyOperation.
 *
 */

#pragma once

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief An operation that can be performed with the key.
   *
   */
  class KeyOperation {
  private:
    std::string m_operation;

  public:
    /**
     * @brief Construct a new Key Operation object.
     *
     * @param operation The operation for the key as string.
     */
    KeyOperation(std::string const& operation) : m_operation(operation) {}

    /**
     * @brief Returns the fully qualified type name of this instance.
     *
     * @return The operation represented as string.
     */
    std::string const& ToString() const { return m_operation; }

    /**
     * @brief The key can be used to encrypt with the #Encrypt(EncryptionAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return Encrypt KeyOperation.
     */
    static KeyOperation Encrypt() { return KeyOperation("encrypt"); }

    /**
     * @brief The key can be used to decrypt with the #Decrypt(EncryptionAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return Decrypt KeyOperation.
     */
    static KeyOperation Decrypt() { return KeyOperation("decrypt"); }

    /**
     * @brief The key can be used to sign with the Sign(SignatureAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return Sign KeyOperation.
     */
    static KeyOperation Sign() { return KeyOperation("sign"); }

    /**
     * @brief The key can be used to verify with the Verify(SignatureAlgorithm, Byte[], Byte[],
     * CancellationToken) method.
     *
     * @return Verify KeyOperation.
     */
    static KeyOperation Verify() { return KeyOperation("verify"); }

    /**
     * @brief The key can be used to wrap another key with the WrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return WrapKey KeyOperation.
     */
    static KeyOperation WrapKey() { return KeyOperation("wrapKey"); }

    /**
     * @brief The key can be used to unwrap another key with the UnwrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return UnwrapKey KeyOperation.
     */
    static KeyOperation UnwrapKey() { return KeyOperation("unwrapKey"); }

    /**
     * @brief The key can be imported during creation using the ImportKey(ImportKeyOptions,
     * CancellationToken) method.
     *
     * @return Import KeyOperation.
     */
    static KeyOperation Import() { return KeyOperation("import"); }
  };

}}}} // namespace Azure::Security::KeyVault::Keys

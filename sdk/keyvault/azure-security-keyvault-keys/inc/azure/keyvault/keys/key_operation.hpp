// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault KeyOperation.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

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
    explicit KeyOperation(std::string operation) : m_operation(std::move(operation)) {}

    /**
     * @brief Construct a default Key operation.
     *
     */
    KeyOperation() = default;

    /**
     * @brief Enables using the equal operator for key operations.
     *
     * @param other A key operation to be compared.
     */
    bool operator==(const KeyOperation& other) const noexcept
    {
      return m_operation == other.m_operation;
    }

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
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Encrypt;

    /**
     * @brief The key can be used to decrypt with the #Decrypt(EncryptionAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return Decrypt KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Decrypt;

    /**
     * @brief The key can be used to sign with the Sign(SignatureAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return Sign KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Sign;

    /**
     * @brief The key can be used to verify with the Verify(SignatureAlgorithm, Byte[], Byte[],
     * CancellationToken) method.
     *
     * @return Verify KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Verify;

    /**
     * @brief The key can be used to wrap another key with the WrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return WrapKey KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation WrapKey;

    /**
     * @brief The key can be used to unwrap another key with the UnwrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     *
     * @return UnwrapKey KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation UnwrapKey;

    /**
     * @brief The key can be imported during creation using the ImportKey(ImportKeyOptions,
     * CancellationToken) method.
     *
     * @return Import KeyOperation.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Import;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

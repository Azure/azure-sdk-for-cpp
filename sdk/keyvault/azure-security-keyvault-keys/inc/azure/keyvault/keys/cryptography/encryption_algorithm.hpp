// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An algorithm used for encryption and decryption.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <stdexcept>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief An algorithm used for encryption and decryption.
   *
   */
  class EncryptionAlgorithm {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new #EncryptionAlgorithm object.
     *
     * @param value The string value of the instance.
     */
    explicit EncryptionAlgorithm(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the encryption algorithm can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    EncryptionAlgorithm() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const EncryptionAlgorithm& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief An RSA1_5 #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm Rsa15;

    /**
     * @brief An RSA-OAEP #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm RsaOaep;

    /**
     * @brief An RSA-OAEP256 #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm RsaOaep256;

    /**
     * @brief An 128-bit AES-GCM #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A128Gcm;

    /**
     * @brief An 192-bit AES-GCM #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A192Gcm;

    /**
     * @brief An 256-bit AES-GCM #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A256Gcm;

    /**
     * @brief An 128-bit AES-CBC #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A128Cbc;

    /**
     * @brief An 192-bit AES-CBC #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A192Cbc;

    /**
     * @brief An 256-bit AES-CBC #EncryptionAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A256Cbc;

    /**
     * @brief An 128-bit AES-CBC #EncryptionAlgorithm with PKCS padding.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A128CbcPad;

    /**
     * @brief An 192-bit AES-CBC #EncryptionAlgorithm with PKCS padding.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A192CbcPad;

    /**
     * @brief An 256-bit AES-CBC #EncryptionAlgorithm with PKCS padding.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const EncryptionAlgorithm A256CbcPad;
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

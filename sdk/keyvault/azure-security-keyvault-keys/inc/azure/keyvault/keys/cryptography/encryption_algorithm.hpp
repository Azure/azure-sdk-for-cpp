// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An algorithm used for encryption and decryption.
 */

#pragma once

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief An algorithm used for encryption and decryption..
   *
   */
  class EncryptionAlgorithm {
  private:
    std::string m_value;

  public:
    EncryptionAlgorithm(std::string value) : m_value(std::move(value)) {}

    std::string const& ToString() { return m_value; }

    /**
     * @brief Gets an RSA1_5 see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm Rsa15();

    /**
     * @brief Gets an RSA-OAEP see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm RsaOaep();

    /**
     * @brief Gets an RSA-OAEP256 see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm RsaOaep256();

    /**
     * @brief Gets a 128-bit AES-GCM see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A128Gcm();

    /**
     * @brief Gets a 192-bit AES-GCM see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A192Gcm();

    /**
     * @brief Gets a 256-bit AES-GCM see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A256Gcm();

    /**
     * @brief Gets a 128-bit AES-CBC see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A128Cbc();

    /**
     * @brief Gets a 192-bit AES-CBC see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A192Cbc();

    /**
     * @brief Gets a 256-bit AES-CBC see #EncryptionAlgorithm.
     *
     */
    static EncryptionAlgorithm A256Cbc();

    /**
     * @brief Gets a 128-bit AES-CBC see #EncryptionAlgorithm> with PKCS padding.
     *
     */
    static EncryptionAlgorithm A128CbcPad();

    /**
     * @brief Gets a 192-bit AES-CBC see #EncryptionAlgorithm> with PKCS padding.
     *
     */
    static EncryptionAlgorithm A192CbcPad();

    /**
     * @brief Gets a 256-bit AES-CBC see #EncryptionAlgorithm> with PKCS padding.
     *
     */
    static EncryptionAlgorithm A256CbcPad();
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

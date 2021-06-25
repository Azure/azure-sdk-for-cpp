// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Parameters for encrypting plaintext.
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
   * @brief Parameters for encrypting plaintext.
   *
   */
  class EncryptParameters final {
  private:
    /**
     * @brief Gets the initialization vector for encryption.
     *
     * @note Initialization vector should not be set for some encryption algorithms. That's why it
     * is private so it is only set by the factory methods.
     *
     */
    std::vector<uint8_t> Iv;

    /**
     * @brief Construct a new Encrypt Parameters object.
     *
     * @param algorithm The #EncryptionAlgorithm to use for encrypt operation.
     * @param plaintext The content to encrypt.
     * @param iv The initialization vector for encryption.
     * @param additionalAuthenticatedData The additional data that is authenticated during
     * decryption but not encrypted.
     */
    EncryptParameters(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> plaintext,
        std::vector<uint8_t> iv,
        std::vector<uint8_t> additionalAuthenticatedData)
        : Iv(std::move(iv)), Algorithm(std::move(algorithm)), Plaintext(std::move(plaintext)),
          AdditionalAuthenticatedData(std::move(additionalAuthenticatedData))
    {
    }

    /**
     * @brief Encrypt Parameters can't be default constructed.
     *
     */
    EncryptParameters() = delete;

  public:
    /**
     * @brief Construct a new Encrypt Parameters object
     *
     * @param algorithm The #EncryptionAlgorithm to use for encrypt operation.
     * @param plaintext The content to encrypt.
     */
    EncryptParameters(EncryptionAlgorithm algorithm, std::vector<uint8_t> const plaintext)
        : Algorithm(std::move(algorithm)), Plaintext(std::move(plaintext))
    {
    }

    /**
     * @brief Gets the #EncryptionAlgorithm.
     *
     */
    EncryptionAlgorithm Algorithm;

    /**
     * @brief Gets the plaintext to encrypt.
     *
     */
    std::vector<uint8_t> Plaintext;

    /**
     * @brief Gets additional data that is authenticated during decryption but not encrypted.
     *
     */
    std::vector<uint8_t> AdditionalAuthenticatedData;

    /**
     * @brief Gets the initialization vector for encryption.
     *
     */
    std::vector<uint8_t> const& GetIv() const { return Iv; }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::Rsa15 encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::Rsa15
     * encryption algorithm.
     */
    static EncryptParameters Rsa15Parameters(std::vector<uint8_t> const& plaintext)
    {
      return EncryptParameters(EncryptionAlgorithm::Rsa15, plaintext);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::RsaOaep encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::RsaOaep
     * encryption algorithm.
     */
    static EncryptParameters RsaOaepParameters(std::vector<uint8_t> const& plaintext)
    {
      return EncryptParameters(EncryptionAlgorithm::RsaOaep, plaintext);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::RsaOaep256 encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::RsaOaep256
     * encryption algorithm.
     */
    static EncryptParameters RsaOaep256Parameters(std::vector<uint8_t> const& plaintext)
    {
      return EncryptParameters(EncryptionAlgorithm::RsaOaep256, plaintext);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A128Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param plaintext The plaintext to encrypt.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A128Gcm
     * encryption algorithm.
     */
    static EncryptParameters A128GcmParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return EncryptParameters(
          EncryptionAlgorithm::A128Gcm, plaintext, {}, additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A192Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param plaintext The plaintext to encrypt.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A192Gcm
     * encryption algorithm.
     */
    static EncryptParameters A192GcmParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return EncryptParameters(
          EncryptionAlgorithm::A192Gcm, plaintext, {}, additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A256Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param plaintext The plaintext to encrypt.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A256Gcm
     * encryption algorithm.
     */
    static EncryptParameters A256GcmParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return EncryptParameters(
          EncryptionAlgorithm::A256Gcm, plaintext, {}, additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A128Cbc encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A128Cbc
     * encryption algorithm.
     */
    static EncryptParameters A128CbcParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A128Cbc, plaintext, iv, {});
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A192Cbc encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A192Cbc
     * encryption algorithm.
     */
    static EncryptParameters A192CbcParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A192Cbc, plaintext, iv, {});
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A256Cbc encryption algorithm.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A192Cbc
     * encryption algorithm.
     */
    static EncryptParameters A256CbcParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A256Cbc, plaintext, iv, {});
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A128CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A128CbcPad
     * encryption algorithm.
     */
    static EncryptParameters A128CbcPadParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A128CbcPad, plaintext, iv, {});
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A192CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A192CbcPad
     * encryption algorithm.
     */
    static EncryptParameters A192CbcPadParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A192CbcPad, plaintext, iv, {});
    }

    /**
     * @brief Creates an instance of the #EncryptParameters class for the
     * #EncryptionAlgorithm::A256CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param plaintext The plaintext to encrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #EncryptParameters class for the #EncryptionAlgorithm::A256CbcPad
     * encryption algorithm.
     */
    static EncryptParameters A256CbcPadParameters(
        std::vector<uint8_t> const& plaintext,
        std::vector<uint8_t> const& iv)
    {
      return EncryptParameters(EncryptionAlgorithm::A256CbcPad, plaintext, iv, {});
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

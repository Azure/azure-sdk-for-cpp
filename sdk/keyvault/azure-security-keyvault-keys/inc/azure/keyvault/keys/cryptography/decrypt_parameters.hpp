// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Parameters for encrypting ciphertext.
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
   * @brief Parameters for encrypting ciphertext.
   *
   */
  class DecryptParameters {
  public:
    EncryptionAlgorithm Algorithm;
    std::vector<uint8_t> Ciphertext;
    std::vector<uint8_t> Iv;
    std::vector<uint8_t> AdditionalAuthenticatedData;
    std::vector<uint8_t> AuthenticationTag;

  public:
    DecryptParameters(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> ciphertext,
        std::vector<uint8_t> iv,
        std::vector<uint8_t> additionalAuthenticatedData,
        std::vector<uint8_t> authenticationTag)
        : Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext)), Iv(std::move(iv)),
          AdditionalAuthenticatedData(std::move(additionalAuthenticatedData)),
          AuthenticationTag(std::move(authenticationTag))
    {
    }

    DecryptParameters(EncryptionAlgorithm algorithm, std::vector<uint8_t> const ciphertext)
        : Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext))
    {
    }

    DecryptParameters(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> ciphertext,
        std::vector<uint8_t> iv)
        : Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext)), Iv(std::move(iv))
    {
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::Rsa15 encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::Rsa15
     * encryption algorithm.
     */
    static DecryptParameters Rsa15Parameters(std::vector<uint8_t> const& ciphertext)
    {
      return DecryptParameters(EncryptionAlgorithm::Rsa15, ciphertext);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::RsaOaep encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::RsaOaep
     * encryption algorithm.
     */
    static DecryptParameters RsaOaepParameters(std::vector<uint8_t> const& ciphertext)
    {
      return DecryptParameters(EncryptionAlgorithm::RsaOaep, ciphertext);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::RsaOaep256 encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::RsaOaep256
     * encryption algorithm.
     */
    static DecryptParameters RsaOaep256Parameters(std::vector<uint8_t> const& ciphertext)
    {
      return DecryptParameters(EncryptionAlgorithm::RsaOaep256, ciphertext);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A128Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv The initialization vector (or nonce) generated during encryption.
     * @param authenticationTag The authentication tag generated during encryption.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A128Gcm
     * encryption algorithm.
     */
    static DecryptParameters A128GcmParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv,
        std::vector<uint8_t> const& authenticationTag,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return DecryptParameters(
          EncryptionAlgorithm::A128Gcm,
          ciphertext,
          iv,
          authenticationTag,
          additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A192Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv The initialization vector (or nonce) generated during encryption.
     * @param authenticationTag The authentication tag generated during encryption.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A192Gcm
     * encryption algorithm.
     */
    static DecryptParameters A192GcmParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv,
        std::vector<uint8_t> const& authenticationTag,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return DecryptParameters(
          EncryptionAlgorithm::A192Gcm,
          ciphertext,
          iv,
          authenticationTag,
          additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A256Gcm encryption algorithm. The nonce will be generated automatically
     * and returned in the #EncryptResult after encryption.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv The initialization vector (or nonce) generated during encryption.
     * @param authenticationTag The authentication tag generated during encryption.
     * @param additionalAuthenticationData Optional data that is authenticated but not encrypted.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A256Gcm
     * encryption algorithm.
     */
    static DecryptParameters A256GcmParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv,
        std::vector<uint8_t> const& authenticationTag,
        std::vector<uint8_t> const& additionalAuthenticationData = {})
    {
      return DecryptParameters(
          EncryptionAlgorithm::A256Gcm,
          ciphertext,
          iv,
          authenticationTag,
          additionalAuthenticationData);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A128Cbc encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A128Cbc
     * encryption algorithm.
     */
    static DecryptParameters A128CbcParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A128Cbc, ciphertext, iv);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A192Cbc encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A192Cbc
     * encryption algorithm.
     */
    static DecryptParameters A192CbcParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A192Cbc, ciphertext, iv);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A256Cbc encryption algorithm.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A192Cbc
     * encryption algorithm.
     */
    static DecryptParameters A256CbcParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A256Cbc, ciphertext, iv);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A128CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A128CbcPad
     * encryption algorithm.
     */
    static DecryptParameters A128CbcPadParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A128CbcPad, ciphertext, iv);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A192CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A192CbcPad
     * encryption algorithm.
     */
    static DecryptParameters A192CbcPadParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A192CbcPad, ciphertext, iv);
    }

    /**
     * @brief Creates an instance of the #DecryptParameters class for the
     * #EncryptionAlgorithm::A256CbcPad encryption algorithm with PKCS#7 padding.
     *
     * @param ciphertext The ciphertext to decrypt.
     * @param iv Optional initialization vector. If null, a cryptographically random initialization
     * vector will be generated using #RandomNumberGenerator.
     * @return An instance of the #DecryptParameters class for the #EncryptionAlgorithm::A256CbcPad
     * encryption algorithm.
     */
    static DecryptParameters A256CbcPadParameters(
        std::vector<uint8_t> const& ciphertext,
        std::vector<uint8_t> const& iv)
    {
      return DecryptParameters(EncryptionAlgorithm::A256CbcPad, ciphertext, iv);
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

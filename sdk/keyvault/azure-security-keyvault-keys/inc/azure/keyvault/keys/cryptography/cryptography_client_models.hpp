// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Parameters for decrypting ciphertext.
 *
 */

#pragma once

#include <azure/core/cryptography/hash.hpp>

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief An algorithm used for signing and verification.
   *
   */
  class SignatureAlgorithm final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new #SignatureAlgorithm object.
     *
     * @param value The string value of the instance.
     */
    explicit SignatureAlgorithm(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the signature algorithm can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default signature algorithm.
     *
     */
    SignatureAlgorithm() = default;

    /**
     * @brief Enables using the equal operator for signature algorithm.
     *
     * @param other A signature algorithm to be compared.
     */
    bool operator==(const SignatureAlgorithm& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the signature algorithm.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Get the Hash Algorithm associated with the #SignatureAlgorithm.
     *
     */
    std::unique_ptr<Azure::Core::Cryptography::Hash> GetHashAlgorithm() const;

    /**
     * @brief An RSA SHA-256 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm RS256;

    /**
     * @brief An RSA SHA-384 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm RS384;

    /**
     * @brief An RSA SHA-512 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm RS512;

    /**
     * cspell: disable-next-line
     * @brief An RSASSA-PSS using SHA-256 and MGF1 with SHA-256 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm PS256;

    /**
     * cspell: disable-next-line
     * @brief An RSASSA-PSS using SHA-384 and MGF1 with SHA-384 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm PS384;

    /**
     * cspell: disable-next-line
     * @brief An RSASSA-PSS using SHA-512 and MGF1 with SHA-512 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm PS512;

    /**
     * @brief An ECDSA with a P-256 curve #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm ES256;

    /**
     * @brief An ECDSA with a P-384 curve #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm ES384;

    /**
     * @brief An ECDSA with a P-512 curve #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm ES512;

    /**
     * cspell: disable-next-line
     * @brief An ECDSA with a secp256k1 curve #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm ES256K;
  };

  /**
   * @brief An algorithm used for key wrap and unwrap.
   *
   */
  class KeyWrapAlgorithm final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new #KeyWrapAlgorithm object.
     *
     * @param value The string value of the instance.
     */
    explicit KeyWrapAlgorithm(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the key wrap algorithm can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    KeyWrapAlgorithm() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const KeyWrapAlgorithm& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief An RSA1_5 #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm Rsa15;

    /**
     * @brief An RSA-OAEP #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm RsaOaep;

    /**
     * @brief An RSA-OAEP-256 #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm RsaOaep256;

    /**
     * @brief An AES 128 #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm A128KW;

    /**
     * @brief An AES 192 #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm A192KW;

    /**
     * @brief An AES 256 #KeyWrapAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyWrapAlgorithm A256KW;
  };

  /**
   * @brief An algorithm used for encryption and decryption.
   *
   */
  class EncryptionAlgorithm final {
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
     * @brief Construct a default encryption algorithm.
     *
     */
    EncryptionAlgorithm() = default;

    /**
     * @brief Enables using the equal operator for encryption algorithm.
     *
     * @param other A encryption algorithm to be compared.
     */
    bool operator==(const EncryptionAlgorithm& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the encryption algorithm.
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

  /**
   * @brief Represents information about a sign operation.
   *
   */
  struct SignResult final
  {
    /**
     * @brief Gets the key identifier of the #KeyVaultKey used to sign. This must be stored
     * alongside the #Signature as the same key must be used to verify it.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the signature.
     *
     */
    std::vector<uint8_t> Signature;

    /**
     * @brief Gets the algorithm used to sign. This must be stored alongside the #Signature as the
     * same algorithm must be used to verify it.
     *
     */
    SignatureAlgorithm Algorithm;
  };

  /**
   * @brief Represents information about an unwrap operation.
   *
   */
  struct UnwrapResult final
  {
    /**
     * @brief Gets the key identifier of the #Key used to unwrap.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the unwrapped key.
     *
     */
    std::vector<uint8_t> Key;

    /**
     * @brief Gets the #KeyWrapAlgorithm used. This must be stored alongside the
     * #EncryptedKey as the same algorithm must be used to unwrap it.
     *
     */
    KeyWrapAlgorithm Algorithm;
  };

  /**
   * @brief Represents information about a verify operation.
   *
   */
  struct VerifyResult final
  {
    /**
     * @brief Gets the key identifier of the #KeyVaultKey used to verify.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets a value indicating whether the specified signature is valid.
     *
     */
    bool IsValid;

    /**
     * @brief Gets the #SignatureAlgorithm.
     *
     */
    SignatureAlgorithm Algorithm;
  };

  /**
   * @brief Represents information about a wrap operation.
   *
   */
  struct WrapResult final
  {
    /**
     * @brief Gets the key identifier of the #KeyVaultKey used to encrypt. This must be stored
     * alongside the #Ciphertext as the same key must be used to decrypt it.
     *
     */
    std::string KeyId;

    /**
     * @brief Gets the wrapped key.
     *
     */
    std::vector<uint8_t> EncryptedKey;

    /**
     * @brief Gets the #KeyWrapAlgorithm used. This must be stored alongside the
     * #EncryptedKey as the same algorithm must be used to unwrap it.
     *
     */
    KeyWrapAlgorithm Algorithm;
  };

  /**
   * @brief Represents information about an encryption operation.
   *
   */
  struct DecryptResult final
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
    std::vector<uint8_t> Plaintext;

    /**
     * @brief Gets the #EncryptionAlgorithm used for encryption. This must be stored alongside the
     * #Ciphertext as the same algorithm must be used to decrypt it.
     *
     */
    EncryptionAlgorithm Algorithm;
  };

  /**
   * @brief Represents information about an encryption operation.
   *
   */
  struct EncryptResult final
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

  /**
   * @brief Parameters for decrypting ciphertext.
   *
   */
  class DecryptParameters final {
  private:
    /**
     * @brief Construct a new Decrypt Parameters object.
     *
     * @param algorithm The #EncryptionAlgorithm to use for decrypt operation.
     * @param ciphertext The content to decrypt.
     * @param iv The initialization vector for decryption.
     * @param additionalAuthenticatedData The additional data that is authenticated during
     * decryption but not encrypted.
     * @param authenticationTag The authenticated tag resulting from encryption with a symmetric key
     * using AES.
     */
    DecryptParameters(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> ciphertext,
        std::vector<uint8_t> iv,
        std::vector<uint8_t> additionalAuthenticatedData,
        std::vector<uint8_t> authenticationTag)
        : m_iv(std::move(iv)), Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext)),
          AdditionalAuthenticatedData(std::move(additionalAuthenticatedData)),
          AuthenticationTag(std::move(authenticationTag))
    {
    }

    DecryptParameters(
        EncryptionAlgorithm algorithm,
        std::vector<uint8_t> ciphertext,
        std::vector<uint8_t> iv)
        : m_iv(std::move(iv)), Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext))
    {
    }

    /**
     * @brief A default DecryptParameters can't be created.
     *
     */
    DecryptParameters() = delete;

    /**
     * @brief Gets the initialization vector for decryption.
     *
     */
    std::vector<uint8_t> m_iv;

  public:
    /**
     * @brief Construct a new Decrypt Parameters object
     *
     * @param algorithm The #EncryptionAlgorithm to use for decrypt operation.
     * @param ciphertext The content to decrypt.
     */
    DecryptParameters(EncryptionAlgorithm algorithm, std::vector<uint8_t> const ciphertext)
        : Algorithm(std::move(algorithm)), Ciphertext(std::move(ciphertext))
    {
    }

    /**
     * @brief Gets or sets the #EncryptionAlgorithm.
     *
     */
    EncryptionAlgorithm Algorithm;

    /**
     * @brief Gets the ciphertext to decrypt.
     *
     */
    std::vector<uint8_t> Ciphertext;

    /**
     * @brief Gets the initialization vector for decryption.
     *
     */
    std::vector<uint8_t> const& GetIv() const { return m_iv; }

    /**
     * @brief Gets additional data that is authenticated during decryption but not encrypted.
     *
     */
    std::vector<uint8_t> AdditionalAuthenticatedData;

    /**
     * @brief Gets the authenticated tag resulting from encryption with a symmetric key using AES.
     *
     */
    std::vector<uint8_t> AuthenticationTag;

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
    std::vector<uint8_t> m_iv;

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
        : m_iv(std::move(iv)), Algorithm(std::move(algorithm)), Plaintext(std::move(plaintext)),
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
    std::vector<uint8_t> const& GetIv() const { return m_iv; }

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
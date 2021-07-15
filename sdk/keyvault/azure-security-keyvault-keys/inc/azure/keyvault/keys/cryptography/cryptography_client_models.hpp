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
     * @brief An RSASSA-PSS using SHA-256 and MGF1 with SHA-256 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm PS256;

    /**
     * @brief An RSASSA-PSS using SHA-384 and MGF1 with SHA-384 #SignatureAlgorithm
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const SignatureAlgorithm PS384;

    /**
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
     * @brief Gets the key identifier of the #Key used to uwrap.
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

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

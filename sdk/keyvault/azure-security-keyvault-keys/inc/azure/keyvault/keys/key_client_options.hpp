// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Keys client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/keys/dll_import_export.hpp"
#include "azure/keyvault/keys/key_client_models.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeys.
   *
   */
  struct GetPropertiesOfKeysOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeyVersions.
   *
   */
  struct GetPropertiesOfKeyVersionsOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };

  /**
   * @brief The options for calling an operation #GetDeletedKeys.
   *
   */
  struct GetDeletedKeysOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct KeyClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Service Version used.
     *
     */
    std::string ApiVersion = "7.3";
  };

  /**
   * @brief Optional parameters for KeyVaultClient::GetKey
   *
   */
  struct GetKeyOptions final
  {
    /**
     * @brief Specify the key version to get.
     *
     */
    std::string Version;
  };

  /**
   * @brief Define the specific options for the #CreateKey operaion.
   *
   */
  struct CreateKeyOptions
  {
    /**
     * @brief Destructor.
     *
     */
    virtual ~CreateKeyOptions() = default;

    /**
     * @brief Define the supported operations for the key.
     *
     */
    std::vector<KeyOperation> KeyOperations;

    /**
     * @brief Indicate when the key will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicate when the key will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief Indicate whether the key is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Specific metadata about the key.
     *
     */
    std::unordered_map<std::string, std::string> Tags;

    /**
     * @brief The policy rules under which the key can be exported.
     *
     */
    Azure::Nullable<KeyReleasePolicy> ReleasePolicy;

    /**
     * @brief Indicate if the private key can be exported.
     *
     */
    Azure::Nullable<bool> Exportable;
  };

  /**
   * @brief The properties needed to create an Elliptic Curve key.
   *
   */
  class CreateEcKeyOptions final : public CreateKeyOptions {
  private:
    std::string m_name;
    bool m_hardwareProtected;
    KeyVaultKeyType m_keyType;

  public:
    /**
     * @brief Gets or sets the elliptic curve name.
     *
     * @remark See #KeyCurveName for possible values.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<KeyCurveName> CurveName;

    /**
     * @brief Create a Ec Key Options object.
     *
     * @param name Name of the key to create.
     * @param hardwareProtected `true` to create hardware-protected key in a hardware security
     * module (HSM). The default is false to create a software key.
     */
    explicit CreateEcKeyOptions(std::string const& name, bool hardwareProtected = false)
        : m_hardwareProtected(hardwareProtected)
    {
      if (name.empty())
      {
        throw std::invalid_argument("The name can't be empty");
      }
      m_name = name;
      if (hardwareProtected)
      {
        m_keyType = KeyVaultKeyType::EcHsm;
      }
      else
      {
        m_keyType = KeyVaultKeyType::Ec;
      }
    }

    /**
     * @brief Gets the name of the key to create.
     *
     */
    std::string const& GetName() const { return m_name; }

    /**
     * @brief Gets the key type to create, including Ec and EcHsm.
     *
     */
    KeyVaultKeyType GetKeyType() const { return m_keyType; }

    /**
     * @brief Gets a value indicating whether to create a hardware-protected key in a hardware
     * security module (HSM).
     *
     */
    bool GetHardwareProtected() const { return m_hardwareProtected; }
  };

  /**
   * @brief The properties needed to create an RSA key.
   *
   */
  class CreateRsaKeyOptions final : public CreateKeyOptions {
  private:
    std::string m_name;
    bool m_hardwareProtected;
    KeyVaultKeyType m_keyType;

  public:
    /**
     * @brief Gets or sets the key size in bits, such as 2048, 3072, or 4096.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<int64_t> KeySize;

    /**
     * @brief Gets or sets the public exponent for a RSA key.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<int64_t> PublicExponent;

    /**
     * @brief Create a RSA Key Options object.
     *
     * @param name Name of the key to create.
     * @param hardwareProtected `true` to create hardware-protected key in a hardware security
     * module (HSM). The default is false to create a software key.
     */
    explicit CreateRsaKeyOptions(std::string const& name, bool hardwareProtected = false)
        : m_hardwareProtected(hardwareProtected)
    {
      if (name.empty())
      {
        throw std::invalid_argument("The name can't be empty");
      }
      m_name = name;
      if (hardwareProtected)
      {
        m_keyType = KeyVaultKeyType::RsaHsm;
      }
      else
      {
        m_keyType = KeyVaultKeyType::Rsa;
      }
    }

    /**
     * @brief Gets the name of the key to create.
     *
     */
    std::string const& GetName() const { return m_name; }

    /**
     * @brief Gets the key type to create, including Rsa and RsaHsm.
     *
     */
    KeyVaultKeyType GetKeyType() const { return m_keyType; }

    /**
     * @brief Gets a value indicating whether to create a hardware-protected key in a hardware
     * security module (HSM).
     *
     */
    bool GetHardwareProtected() const { return m_hardwareProtected; }
  };

  /**
   * @brief The properties needed to create an AES key.
   *
   */
  class CreateOctKeyOptions final : public CreateKeyOptions {
  private:
    std::string m_name;
    bool m_hardwareProtected;
    KeyVaultKeyType m_keyType;

  public:
    /**
     * @brief Gets or sets the key size in bits, such as 2048, 3072, or 4096.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<int64_t> KeySize;

    /**
     * @brief Create a AES Key Options object.
     *
     * @param name Name of the key to create.
     * @param hardwareProtected `true` to create hardware-protected key in a hardware security
     * module (HSM). The default is false to create a software key.
     */
    explicit CreateOctKeyOptions(std::string const& name, bool hardwareProtected = false)
        : m_hardwareProtected(hardwareProtected)
    {
      if (name.empty())
      {
        throw std::invalid_argument("The name can't be empty");
      }
      m_name = name;
      if (hardwareProtected)
      {
        m_keyType = KeyVaultKeyType::OctHsm;
      }
      else
      {
        m_keyType = KeyVaultKeyType::Oct;
      }
    }

    /**
     * @brief Gets the name of the key to create.
     *
     */
    std::string const& GetName() const { return m_name; }

    /**
     * @brief Gets the key type to create, including Oct and OctHsm.
     *
     */
    KeyVaultKeyType GetKeyType() const { return m_keyType; }

    /**
     * @brief Gets a value indicating whether to create a hardware-protected key in a hardware
     * security module (HSM).
     *
     */
    bool GetHardwareProtected() const { return m_hardwareProtected; }
  };

  /**
   * @brief A key resource and its properties.
   *
   */
  struct ImportKeyOptions final
  {
    /**
     * @brief The cryptographic key, the key type, and the operations you can perform using the key.
     *
     */
    JsonWebKey Key;

    /**
     * @brief The additional properties.
     *
     */
    KeyProperties Properties;

    /**
     * @brief Get or Set a value indicating whether to import the key into a hardware security
     * module (HSM).
     *
     */
    Azure::Nullable<bool> HardwareProtected;

    /**
     * @brief Construct a new Key Vault ImportKeyOptions object.
     *
     * @param name The name of the key.
     */
    ImportKeyOptions(std::string name, JsonWebKey keyMaterial)
        : Key(keyMaterial), Properties(std::move(name))
    {
    }

    /**
     * @brief Gets the name of the Key.
     *
     * @return The name of the key.
     */
    std::string const& Name() const { return Properties.Name; }
  };

  /**
   * @brief Get Random Bytes options
   *
   */
  struct GetRandomBytesOptions final
  {
    /**
     * @brief The requested number of random bytes.
     *
     */
    int32_t Count;
  };

  /**
   * @brief Release key options.
   *
   */
  struct KeyReleaseOptions final
  {
    /**
     * @brief The attestation assertion for the target of the key release.
     *
     */
    std::string Target;

    /**
     * @brief A client provided nonce for freshness.
     *
     */
    Azure::Nullable<std::string> Nonce;

    /**
     * @brief The encryption algorithm to use to protected the exported key material.
     *
     */
    Azure::Nullable<KeyEncryptionAlgorithm> Encryption;

    /**
     * @brief The version of the key to release.
     *
     */
    Azure::Nullable<std::string> Version;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

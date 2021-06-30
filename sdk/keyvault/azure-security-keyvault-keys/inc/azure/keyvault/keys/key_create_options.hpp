// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Key.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_operation_type.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <list>
#include <string>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

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
    std::list<KeyOperationType> KeyOperations;

    /**
     * @brief Indicates when the key will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicates when the key will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief whether the key is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Specific metadata about the key.
     *
     */
    std::unordered_map<std::string, std::string> Tags;
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
    std::string const& GetName() const { return m_name; };

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
    std::string const& GetName() const { return m_name; };

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
    std::string const& GetName() const { return m_name; };

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

}}}} // namespace Azure::Security::KeyVault::Keys

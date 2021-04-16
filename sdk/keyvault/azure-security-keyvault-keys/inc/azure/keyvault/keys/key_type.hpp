// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey types.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The JsonWebKey types.
   *
   */
  class KeyVaultKeyType {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new JSON Web Token (JWT) type object.
     *
     * @param jwt The JWT as a string.
     */
    explicit KeyVaultKeyType(std::string jwt) : m_value(std::move(jwt)) {}

    /**
     * @brief Construct a default KeyVaultKeyType with an empty string.
     *
     */
    KeyVaultKeyType() = default;

    /**
     * @brief Enables using the equal operator for JWT.
     *
     * @param other A JWT to be compared.
     */
    bool operator==(const KeyVaultKeyType& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Return the JWT as string.
     *
     * @return The JWT represented as string.
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Ec;

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm backed by a Hardware Security Module
     * (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType EcHsm;

    /**
     * @brief An RSA cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Rsa;

    /**
     * @brief An RSA cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType RsaHsm;

    /**
     * @brief An AES cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Oct;

    /**
     * @brief An AES cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType OctHsm;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

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
  class JsonWebKeyType {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new JWT Type object
     *
     * @param jwt The JWT as string.
     */
    explicit JsonWebKeyType(std::string jwt) : m_value(std::move(jwt)) {}

    /**
     * @brief Construct a default JWT.
     *
     */
    JsonWebKeyType() = default;

    /**
     * @brief Enables using the equal operator for JWT.
     *
     * @param other A JWT to be compared.
     */
    bool operator==(const JsonWebKeyType& other) const noexcept { return m_value == other.m_value; }

    /**
     * @brief Return the JWK as string.
     *
     * @return The JWK represented as string.
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType Ec;

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm backed by a Hardware Security Module
     * (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType EcHsm;

    /**
     * @brief An RSA cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType Rsa;

    /**
     * @brief An RSA cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType RsaHsm;

    /**
     * @brief An AES cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType Oct;

    /**
     * @brief An AES cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const JsonWebKeyType OctHsm;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

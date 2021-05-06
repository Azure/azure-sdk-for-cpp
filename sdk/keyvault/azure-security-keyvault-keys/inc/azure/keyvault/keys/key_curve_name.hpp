// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Elliptic Curve Cryptography (ECC) curve names.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Elliptic Curve Cryptography (ECC) curve names.
   *
   */
  class KeyCurveName {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit KeyCurveName(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the curve name can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    KeyCurveName() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const KeyCurveName& other) const noexcept { return m_value == other.m_value; }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Gets the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P256;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P256K;

    /**
     * @brief Gets the NIST P-384 elliptic curve, AKA SECG curve SECP384R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P384;

    /**
     * @brief Gets the NIST P-521 elliptic curve, AKA SECG curve SECP521R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P521;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

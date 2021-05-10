// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An algorithm used for key wrap and unwrap.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <stdexcept>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

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
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

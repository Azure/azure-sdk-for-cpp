// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An algorithm used for signing and verification.
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
   * @brief An algorithm used for signing and verification.
   *
   */
  class SignatureAlgorithm {
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
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey types.
 *
 */

#pragma once

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief The JsonWebKey types.
   *
   */
  enum class JsonWebKeyType
  {
    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm.
     *
     */
    Ec,
    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm backed by a Hardware Security Module
     * (HSM).
     *
     */
    EcHsm,
    /**
     * @brief An RSA cryptographic algorithm.
     *
     */
    Rsa,
    /**
     * @brief An RSA cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    RsaHsm,
    /**
     * @brief An AES cryptographic algorithm.
     *
     */
    Oct,
    /**
     * @brief An AES cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    OctHsm,
  };

  namespace Details {
    JsonWebKeyType KeyTypeFromString(std::string const& name);
    std::string KeyTypeToString(JsonWebKeyType kty);
  } // namespace Details

}}}} // namespace Azure::Security::KeyVault::Keys

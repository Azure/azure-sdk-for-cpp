// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about a sign operation.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/signature_algorithm.hpp"

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

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

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

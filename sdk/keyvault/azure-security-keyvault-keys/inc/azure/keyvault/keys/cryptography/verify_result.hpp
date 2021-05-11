// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents information about a verify operation.
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
   * @brief Represents information about a verify operation.
   *
   */
  struct VerifyResult
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

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

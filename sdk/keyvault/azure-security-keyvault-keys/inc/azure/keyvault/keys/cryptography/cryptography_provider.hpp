// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the functional contract for a cryptography provider.
 *
 */

#include <azure/core/context.hpp>

#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/key_operation.hpp"

#pragma once

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Defines the functional contract for a cryptography provider.
   *
   */
  struct ICryptographyProvider
  {
    virtual bool CanRemote() const = 0;

    virtual bool SupportsOperation(KeyOperation const& operation) const = 0;

    virtual EncryptResult Encrypt(
        EncryptParameters parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const = 0;

    virtual DecryptResult Decrypt(
        DecryptParameters parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const = 0;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

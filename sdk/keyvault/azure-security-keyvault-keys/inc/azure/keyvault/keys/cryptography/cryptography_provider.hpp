// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/context.hpp>

#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/key_operation.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct CryptographyProvider
    {
      virtual ~CryptographyProvider() = default;

      virtual bool CanRemote() const = 0;

      virtual bool SupportsOperation(Azure::Security::KeyVault::Keys::KeyOperation operation) = 0;

      virtual EncryptResult Encrypt(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context) const = 0;

      virtual DecryptResult Decrypt(
          DecryptParameters const& parameters,
          Azure::Core::Context const& context) const = 0;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

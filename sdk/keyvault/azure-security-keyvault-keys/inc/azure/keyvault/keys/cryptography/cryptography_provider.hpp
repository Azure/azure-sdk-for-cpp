// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the operations supported by a Cryptography provider.
 *
 */

#pragma once

#include <azure/core/context.hpp>

#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/key_wrap_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/unwrap_result.hpp"
#include "azure/keyvault/keys/cryptography/wrap_result.hpp"
#include "azure/keyvault/keys/key_operation.hpp"

#include <memory>
#include <string>
#include <vector>

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

      virtual WrapResult WrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& key,
          Azure::Core::Context const& context) const = 0;

      virtual UnwrapResult UnwrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& encryptedKey,
          Azure::Core::Context const& context) const = 0;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

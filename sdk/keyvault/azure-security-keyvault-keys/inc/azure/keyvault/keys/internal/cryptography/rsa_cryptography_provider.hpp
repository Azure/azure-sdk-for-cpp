// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief RSA local cryptography provider.
 *
 */

#pragma once

#include "azure/keyvault/keys/internal/cryptography/local_cryptography_provider.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct RsaCryptographyProvider final : public LocalCryptographyProvider
    {
      RsaCryptographyProvider(
          Azure::Security::KeyVault::Keys::JsonWebKey const& keyMaterial,
          Azure::Security::KeyVault::Keys::KeyProperties const& keyProperties,
          bool localOnly)
          : LocalCryptographyProvider(keyMaterial, keyProperties, localOnly)
      {
      }

      bool SupportsOperation(
          Azure::Security::KeyVault::Keys::KeyOperationType operation) const noexcept override
      {
        if (operation == Azure::Security::KeyVault::Keys::KeyOperationType::Encrypt
            || operation == Azure::Security::KeyVault::Keys::KeyOperationType::Decrypt
            || operation == Azure::Security::KeyVault::Keys::KeyOperationType::Sign
            || operation == Azure::Security::KeyVault::Keys::KeyOperationType::Verify
            || operation == Azure::Security::KeyVault::Keys::KeyOperationType::WrapKey
            || operation == Azure::Security::KeyVault::Keys::KeyOperationType::UnwrapKey)
        {
          return m_keyMaterial.SupportsOperation(operation);
        }
        return false;
      }

      EncryptResult Encrypt(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context) const override;

      DecryptResult Decrypt(
          DecryptParameters const& parameters,
          Azure::Core::Context const& context) const override;

      WrapResult WrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& key,
          Azure::Core::Context const& context) const override;

      UnwrapResult UnwrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& encryptedKey,
          Azure::Core::Context const& context) const override;

      SignResult Sign(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& sigest,
          Azure::Core::Context const& context) const override;

      VerifyResult Verify(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& sigest,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context) const override;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/local_cryptography_provider.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct RsaCryptographyProvider : public LocalCryptographyProvider
    {
      RsaCryptographyProvider(
          Azure::Security::KeyVault::Keys::JsonWebKey const& keyMaterial,
          Azure::Security::KeyVault::Keys::KeyProperties const& keyProperties,
          bool localOnly)
          : LocalCryptographyProvider(keyMaterial, keyProperties, localOnly)
      {
      }

      bool SupportsOperation(Azure::Security::KeyVault::Keys::KeyOperation operation) override
      {
        if (operation == Azure::Security::KeyVault::Keys::KeyOperation::Encrypt
            || operation == Azure::Security::KeyVault::Keys::KeyOperation::Decrypt
            || operation == Azure::Security::KeyVault::Keys::KeyOperation::Sign
            || operation == Azure::Security::KeyVault::Keys::KeyOperation::Verify
            || operation == Azure::Security::KeyVault::Keys::KeyOperation::WrapKey
            || operation == Azure::Security::KeyVault::Keys::KeyOperation::UnwrapKey)
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
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

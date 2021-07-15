// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Exposes a factory for creating local cryptography providers.
 *
 */

#pragma once

#include "azure/keyvault/keys/keyvault_key.hpp"

#include "cryptography_provider.hpp"
#include "rsa_cryptography_provider.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct LocalCryptographyProviderFactory final
    {
      LocalCryptographyProviderFactory() = delete;

      static std::unique_ptr<CryptographyProvider> Create(
          Azure::Security::KeyVault::Keys::JsonWebKey const& keyMaterial,
          Azure::Security::KeyVault::Keys::KeyProperties const& keyProperties,
          bool localOnly = false)
      {
        if (keyMaterial.KeyType == Azure::Security::KeyVault::Keys::KeyVaultKeyType::Rsa
            || keyMaterial.KeyType == Azure::Security::KeyVault::Keys::KeyVaultKeyType::RsaHsm)
        {
          return std::make_unique<RsaCryptographyProvider>(keyMaterial, keyProperties, localOnly);
        }
        return nullptr;
      }

      static std::unique_ptr<CryptographyProvider> Create(
          Azure::Security::KeyVault::Keys::KeyVaultKey const& key,
          bool localOnly = false)
      {
        return Create(key.Key, key.Properties, localOnly);
      }
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

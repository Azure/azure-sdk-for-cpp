// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/cryptography_provider.hpp"
#include "azure/keyvault/keys/cryptography/rsa_cryptography_provider.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct LocalCryptographyProviderFactory
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

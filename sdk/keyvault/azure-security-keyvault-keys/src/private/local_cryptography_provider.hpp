// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the base bahavior for a local cryptography provider.
 *
 */

#pragma once

#include "cryptography_provider.hpp"

#include "azure/keyvault/keys/key_client_models.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    class LocalCryptographyProvider : public CryptographyProvider {
    private:
      Azure::Security::KeyVault::Keys::KeyProperties m_keyProperties;
      bool m_canRemote;

    protected:
      Azure::Security::KeyVault::Keys::JsonWebKey m_keyMaterial;

    public:
      LocalCryptographyProvider() = delete;

      LocalCryptographyProvider(
          Azure::Security::KeyVault::Keys::JsonWebKey const& keyMaterial,
          Azure::Security::KeyVault::Keys::KeyProperties const& keyProperties,
          bool localOnly = false)
          : m_keyProperties(keyProperties), m_canRemote(!localOnly && !keyMaterial.Id.empty()),
            m_keyMaterial(keyMaterial)
      {
      }

      bool CanRemote() const noexcept override { return m_canRemote; };
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

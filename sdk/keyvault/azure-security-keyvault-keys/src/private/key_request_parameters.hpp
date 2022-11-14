//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal implementation for sending the HTTP request.
 *
 */

#pragma once

#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_client_models.hpp"
#include "azure/keyvault/keys/key_client_options.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {

  class KeyRequestParameters final : public Azure::Core::Json::_internal::JsonSerializable {
  private:
    Azure::Nullable<KeyVaultKeyType> m_keyType;
    CreateKeyOptions m_options;

  public:
    Azure::Nullable<KeyCurveName> Curve;
    Azure::Nullable<int64_t> KeySize;
    Azure::Nullable<int64_t> PublicExponent;

    explicit KeyRequestParameters(
        KeyProperties const& key,
        Azure::Nullable<std::vector<KeyOperation>> const& operations)
        : m_options(CreateKeyOptions())
    {
      if (key.Enabled)
      {
        m_options.Enabled = key.Enabled.Value();
      }
      if (key.ExpiresOn)
      {
        m_options.ExpiresOn = key.ExpiresOn.Value();
      }
      if (key.NotBefore)
      {
        m_options.NotBefore = key.NotBefore.Value();
      }
      if (key.Tags.size() > 0)
      {
        m_options.Tags = std::unordered_map<std::string, std::string>(key.Tags);
      }
      if (key.ReleasePolicy)
      {
        m_options.ReleasePolicy = key.ReleasePolicy;
      }
      if (key.Exportable)
      {
        m_options.Exportable = key.Exportable.Value();
      }
      if (operations)
      {
        m_options.KeyOperations = std::vector<KeyOperation>(operations.Value());
      }
    }

    explicit KeyRequestParameters(KeyVaultKeyType keyType, CreateKeyOptions const& options)
        : m_keyType(keyType), m_options(options)
    {
    }

    explicit KeyRequestParameters(CreateEcKeyOptions const& ecKey)
        : KeyRequestParameters(ecKey.GetKeyType(), ecKey)
    {
      if (ecKey.CurveName.HasValue())
      {
        Curve = ecKey.CurveName.Value();
      }
    }

    explicit KeyRequestParameters(CreateRsaKeyOptions const& rsaKey)
        : KeyRequestParameters(rsaKey.GetKeyType(), rsaKey)
    {
      if (rsaKey.KeySize.HasValue())
      {
        KeySize = rsaKey.KeySize.Value();
      }
      if (rsaKey.PublicExponent.HasValue())
      {
        PublicExponent = rsaKey.PublicExponent.Value();
      }
    }

    explicit KeyRequestParameters(CreateOctKeyOptions const& octKey)
        : KeyRequestParameters(octKey.GetKeyType(), octKey)
    {
      if (octKey.KeySize.HasValue())
      {
        KeySize = octKey.KeySize.Value();
      }
    }

    std::string Serialize() const override;
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail

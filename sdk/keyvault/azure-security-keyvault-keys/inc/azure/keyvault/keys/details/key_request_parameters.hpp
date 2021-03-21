// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal implementation for sending the HTTP request.
 *
 */

#pragma once

#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_create_options.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {

  class KeyRequestParameters : public Azure::Core::Json::_internal::JsonSerializable {
  private:
    Azure::Nullable<JsonWebKeyType> m_keyType;
    CreateKeyOptions m_options;

  public:
    Azure::Nullable<KeyCurveName> Curve;
    Azure::Nullable<uint64_t> KeySize;
    Azure::Nullable<uint64_t> PublicExponent;

    explicit KeyRequestParameters(
        KeyProperties const& key,
        Azure::Nullable<std::list<KeyOperation>> const& operations)
        : m_options(CreateKeyOptions())
    {
      if (key.Enabled)
      {
        m_options.Enabled = key.Enabled.GetValue();
      }
      if (key.ExpiresOn)
      {
        m_options.ExpiresOn = key.ExpiresOn.GetValue();
      }
      if (key.NotBefore)
      {
        m_options.NotBefore = key.NotBefore.GetValue();
      }
      if (key.Tags.size() > 0)
      {
        m_options.Tags = std::unordered_map<std::string, std::string>(key.Tags);
      }
      if (operations)
      {
        m_options.KeyOperations = std::list<KeyOperation>(operations.GetValue());
      }
    }

    explicit KeyRequestParameters(JsonWebKeyType keyType, CreateKeyOptions const& options)
        : m_keyType(keyType), m_options(options)
    {
    }

    explicit KeyRequestParameters(CreateEcKeyOptions const& ecKey)
        : KeyRequestParameters(ecKey.GetKeyType(), ecKey)
    {
      if (ecKey.CurveName.HasValue())
      {
        Curve = ecKey.CurveName.GetValue();
      }
    }

    explicit KeyRequestParameters(CreateRsaKeyOptions const& rsaKey)
        : KeyRequestParameters(rsaKey.GetKeyType(), rsaKey)
    {
      if (rsaKey.KeySize.HasValue())
      {
        KeySize = rsaKey.KeySize.GetValue();
      }
      if (rsaKey.PublicExponent.HasValue())
      {
        PublicExponent = rsaKey.PublicExponent.GetValue();
      }
    }

    explicit KeyRequestParameters(CreateOctKeyOptions const& octKey)
        : KeyRequestParameters(octKey.GetKeyType(), octKey)
    {
      if (octKey.KeySize.HasValue())
      {
        KeySize = octKey.KeySize.GetValue();
      }
    }

    std::string Serialize() const override;
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail

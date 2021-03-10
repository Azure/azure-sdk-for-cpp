// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal implementation for sending the HTTP request.
 *
 */

#pragma once

#include <azure/core/internal/json_serializable.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_create_options.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {

  class KeyRequestParameters : public Azure::Core::_internal::Json::JsonSerializable {
  private:
    JsonWebKeyType m_keyType;
    CreateKeyOptions const& m_options;

  public:
    Azure::Core::Nullable<KeyCurveName> Curve;
    Azure::Core::Nullable<uint64_t> KeySize;
    Azure::Core::Nullable<uint64_t> PublicExponent;

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

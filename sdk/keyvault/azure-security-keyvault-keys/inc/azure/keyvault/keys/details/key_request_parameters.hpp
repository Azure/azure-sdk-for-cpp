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
#include "azure/keyvault/keys/key_type.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Details {

  class KeyRequestParameters : public Azure::Core::Internal::Json::JsonSerializable {
  private:
    JsonWebKeyType m_keyType;
    CreateKeyOptions const& m_options;

  public:
    explicit KeyRequestParameters(JsonWebKeyType keyType, CreateKeyOptions const& options)
        : m_keyType(keyType), m_options(options)
    {
    }

    std::string Serialize() const override;
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Details

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the JsonWebKey.
 * 
 */

#pragma once

#include "azure/keyvault/keys/key_constants.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  struct JsonWebKey
  {
    /**
     * @brief The Identifier of the key. This is not limited to a Url.
     *
     */
    std::string Id;
    KeyTypeEnum KeyType;

    JsonWebKey() {}

    void SetKeyOperations(std::vector<KeyOperation> const& keyOperations)
    {
      m_keyOps = keyOperations;
    }
    std::vector<KeyOperation> const& KeyOperations() const { return m_keyOps; }

  private:
    std::vector<KeyOperation> m_keyOps;
  };

}}}} // namespace Azure::Security::KeyVault::Keys

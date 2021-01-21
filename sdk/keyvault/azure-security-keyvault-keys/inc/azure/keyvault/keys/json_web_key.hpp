// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* KeyIdPropertyName = "kid";
    constexpr static const char* KeyTypePropertyName = "kty";
    constexpr static const char* KeyOpsPropertyName = "key_ops";
    constexpr static const char* CurveNamePropertyName = "crv";
    constexpr static const char* NPropertyName = "n";
    constexpr static const char* EPropertyName = "e";
    constexpr static const char* DPPropertyName = "dp";
    constexpr static const char* DQPropertyName = "dq";
    constexpr static const char* QIPropertyName = "qi";
    constexpr static const char* PPropertyName = "p";
    constexpr static const char* QPropertyName = "q";
    constexpr static const char* XPropertyName = "x";
    constexpr static const char* YPropertyName = "y";
    constexpr static const char* DPropertyName = "d";
    constexpr static const char* KPropertyName = "k";
    constexpr static const char* TPropertyName = "key_hsm";
  } // namespace Details

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

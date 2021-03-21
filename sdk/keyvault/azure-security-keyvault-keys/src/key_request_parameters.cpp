// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_request_parameters.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;

std::string KeyRequestParameters::Serialize() const
{

  Azure::Core::Json::_internal::json payload;
  /* Mandatory */
  // kty
  SetFromNullable<JsonWebKeyType>(
      m_keyType, payload, _detail::KeyTypePropertyName, [&](JsonWebKeyType type) {
        return KeyTypeToString(type);
      });

  /* Optional */
  // key_size
  // public_exponent
  // key_ops
  for (KeyOperation op : m_options->KeyOperations)
  {
    payload[_detail::KeyOpsPropertyName].push_back(op.ToString());
  }

  // attributes
  // tags
  for (auto tag : m_options->Tags)
  {
    payload[_detail::TagsPropertyName][tag.first] = tag.second;
  }

  // crv
  if (Curve.HasValue())
  {
    payload[_detail::CurveNamePropertyName] = Curve.GetValue().ToString();
  }

  // release_policy
  return payload.dump();
}

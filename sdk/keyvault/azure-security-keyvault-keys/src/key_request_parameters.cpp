// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_request_parameters.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::_detail;

std::string KeyRequestParameters::Serialize() const
{

  Azure::Core::_internal::Json::json payload;
  /* Mandatory */
  // kty
  payload[Details::KeyTypePropertyName] = KeyTypeToString(m_keyType);

  /* Optional */
  // key_size
  // public_exponent
  // key_ops
  for (KeyOperation op : m_options.KeyOperations)
  {
    payload[Details::KeyOpsPropertyName].push_back(op.ToString());
  }

  // attributes
  // tags
  for (auto tag : m_options.Tags)
  {
    payload[Details::TagsPropertyName][tag.first] = tag.second;
  }

  // crv
  if (Curve.HasValue())
  {
    payload[Details::CurveNamePropertyName] = Curve.GetValue().ToString();
  }

  // release_policy
  return payload.dump();
}

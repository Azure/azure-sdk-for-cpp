// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json.hpp>

#include "azure/keyvault/keys/key_constants.hpp"
#include "azure/keyvault/keys/key_request_parameters.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::Details;

std::string KeyRequestParameters::Serialize() const
{

  Azure::Core::Internal::Json::json payload;
  // kty
  payload[Details::KeyTypePropertyName] = KeyTypeToString(m_keyType);

  return payload.dump();
}

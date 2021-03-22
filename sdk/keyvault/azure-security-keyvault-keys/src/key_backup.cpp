// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/details/key_backup.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;

std::string KeyBackup::Serialize() const
{

  Azure::Core::Json::_internal::json payload;

  payload["value"] = Azure::Core::Convert::Base64Encode(Value);

  // release_policy
  return payload.dump();
}

KeyBackup KeyBackup::Deserialize(Azure::Core::Http::RawResponse const& rawResponse)
{
  auto& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  KeyBackup keyBackup;
  JsonOptional::SetIfExists(keyBackup.Value, jsonParser, "value");
  return keyBackup;
}

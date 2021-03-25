// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <azure/keyvault/common/internal/base64url.hpp>

#include "azure/keyvault/keys/details/key_backup.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Common::_internal;

std::string KeyBackup::Serialize() const
{
  Azure::Core::Json::_internal::json payload;

  payload["value"] = Base64url::Base64UrlEncode(Value);

  // release_policy
  return payload.dump();
}

KeyBackup KeyBackup::Deserialize(Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  KeyBackup keyBackup;
  JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
      keyBackup.Value, jsonParser, "value", [](std::string const& value) {
        return Base64url::Base64UrlDecode(value);
      });
  return keyBackup;
}

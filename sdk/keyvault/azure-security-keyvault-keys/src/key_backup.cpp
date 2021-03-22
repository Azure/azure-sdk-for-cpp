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
  auto base64 = Azure::Core::Convert::Base64Encode(Value);
  // update to base64url
  auto trail = base64.find('=');
  if (trail != std::string::npos)
  {
    base64 = base64.substr(0, trail);
  }
  std::replace(base64.begin(), base64.end(), '+', '-');
  std::replace(base64.begin(), base64.end(), '/', '_');
  payload["value"] = base64;
  // release_policy
  return payload.dump();
}

KeyBackup KeyBackup::Deserialize(Azure::Core::Http::RawResponse const& rawResponse)
{
  auto& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  KeyBackup keyBackup;
  JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
      keyBackup.Value, jsonParser, "value", [](std::string const& value) {
        std::string base64url(value);
        // base64url to base64
        std::replace(base64url.begin(), base64url.end(), '-', '+');
        std::replace(base64url.begin(), base64url.end(), '_', '/');
        switch (base64url.size() % 4)
        {
          case 0:
            break;
          case 2:
            base64url.append("==");
            break;
          case 3:
            base64url.append("=");
            break;
          default:
            throw new std::invalid_argument("Unexpected base64 encoding in the http response.");
        }
        return Azure::Core::Convert::Base64Decode(base64url);
      });
  return keyBackup;
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;

std::string _detail::GetRandomBytesSerializer::GetRandomBytesOptionsSerialize(
    GetRandomBytesOptions const& options)
{
  json payload;

  payload[_detail::CountPropertiesValue] = options.Count;

  return payload.dump();
}

std::vector<uint8_t> _detail::GetRandomBytesSerializer::GetRandomBytesResponseDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto const jsonParser = Azure::Core::Json::_internal::json::parse(body);

  auto value = jsonParser[_detail::ValueParameterValue].get<std::string>();
  return Azure::Core::_internal::Base64Url::Base64UrlDecode(value);
}
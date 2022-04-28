// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_options.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;

Azure::Core::Json::_internal::json
Azure::Security::KeyVault::Keys::_detail::KeyReleasePolicySerializer::KeyReleasePolicySerialize(
    KeyReleasePolicy const& policy)
{
  Azure::Core::Json::_internal::json payload;

  payload[_detail::ContentTypeValue] = policy.ContentType.ValueOr(_detail::ContentTypeDefaultValue);
  payload[_detail::ImmutableValue] = policy.Immutable;
  payload[_detail::DataValue]
      = Base64Url::Base64UrlEncode(std::vector<uint8_t>(policy.Data.begin(), policy.Data.end()));

  return payload;
}

KeyReleasePolicy
Azure::Security::KeyVault::Keys::_detail::KeyReleasePolicySerializer::KeyReleasePolicyDeserialize(
    Azure::Core::Json::_internal::json const& response)
{
  KeyReleasePolicy policy;
  auto decodedData = Base64Url::Base64UrlDecode(response[_detail::DataValue].get<std::string>());

  policy.ContentType = response[_detail::ContentTypeValue].get<std::string>();
  policy.Immutable = response[_detail::ImmutableValue].get<bool>();
  policy.Data = std::string(decodedData.begin(), decodedData.end());

  return policy;
}

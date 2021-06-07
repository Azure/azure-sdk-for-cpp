// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/wrap_result.hpp"

#include <azure/keyvault/common/internal/base64url.hpp>

#include <string>
#include <vector>

using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Security::KeyVault::_internal;

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  WrapResult _detail::WrapResultSerializer::WrapResultDeserialize(
      Azure::Core::Http::RawResponse const& rawResponse)
  {
    auto const& body = rawResponse.GetBody();
    auto jsonParser = json::parse(body);

    WrapResult result;
    result.KeyId = jsonParser[KeyIdPropertyName].get<std::string>();

    if (jsonParser.contains(ValueParameterValue) && !jsonParser[ValueParameterValue].is_null())
    {
      result.EncryptedKey
          = Base64Url::Base64UrlDecode(jsonParser[ValueParameterValue].get<std::string>());
    }

    return result;
  }

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

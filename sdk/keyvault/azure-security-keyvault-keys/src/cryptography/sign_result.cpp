// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

#include <string>
#include <vector>

using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Keys::_detail;

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  SignResult _detail::SignResultSerializer::SignResultDeserialize(
      Azure::Core::Http::RawResponse const& rawResponse)
  {
    auto const& body = rawResponse.GetBody();
    auto jsonParser = json::parse(body);

    SignResult result;
    result.KeyId = jsonParser[KeyIdPropertyName].get<std::string>();
    result.Signature
        = Base64Url::Base64UrlDecode(jsonParser[ValueParameterValue].get<std::string>());

    return result;
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_serializers.hpp"
#include "azure/keyvault/keys/cryptography/sign_result.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

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

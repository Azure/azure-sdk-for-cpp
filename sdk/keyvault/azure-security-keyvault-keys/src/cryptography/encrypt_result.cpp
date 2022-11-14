//  Copyright (c) Microsoft Corporation. All rights reserved.
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

  EncryptResult _detail::EncryptResultSerializer::EncryptResultDeserialize(
      Azure::Core::Http::RawResponse const& rawResponse)
  {
    auto const& body = rawResponse.GetBody();
    auto jsonParser = json::parse(body);

    EncryptResult result;
    result.KeyId = jsonParser[KeyIdPropertyName].get<std::string>();
    result.Ciphertext
        = Base64Url::Base64UrlDecode(jsonParser[ValueParameterValue].get<std::string>());

    if (jsonParser.contains(IvValue) && !jsonParser[IvValue].is_null())
    {
      result.Iv = Base64Url::Base64UrlDecode(jsonParser[IvValue].get<std::string>());
    }

    if (jsonParser.contains(AdditionalAuthenticatedValue)
        && !jsonParser[AdditionalAuthenticatedValue].is_null())
    {
      result.AdditionalAuthenticatedData
          = Base64Url::Base64UrlDecode(jsonParser[AdditionalAuthenticatedValue].get<std::string>());
    }

    if (jsonParser.contains(AuthenticationTagValue)
        && !jsonParser[AuthenticationTagValue].is_null())
    {
      result.AuthenticationTag
          = Base64Url::Base64UrlDecode(jsonParser[AuthenticationTagValue].get<std::string>());
    }

    return result;
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

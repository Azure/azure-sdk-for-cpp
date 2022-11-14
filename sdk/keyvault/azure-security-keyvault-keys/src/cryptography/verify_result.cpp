//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

#include <string>
#include <vector>

using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Keys::_detail;

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  VerifyResult _detail::VerifyResultSerializer::VerifyResultDeserialize(
      Azure::Core::Http::RawResponse const& rawResponse)
  {
    auto const& body = rawResponse.GetBody();
    auto jsonParser = json::parse(body);

    VerifyResult result;
    result.IsValid = jsonParser[ValueParameterValue].get<bool>();

    return result;
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

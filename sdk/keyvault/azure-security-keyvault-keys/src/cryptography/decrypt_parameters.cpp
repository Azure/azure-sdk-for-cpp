// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  std::string _detail::DecryptParametersSerializer::DecryptParametersSerialize(
      DecryptParameters const& parameters)
  {
    Azure::Core::Json::_internal::json payload;

    using namespace Azure::Security::KeyVault::Keys::_detail;
    using namespace Azure::Core::_internal;
    payload[AlgorithmValue] = parameters.Algorithm.ToString();
    payload[ValueParameterValue] = Base64Url::Base64UrlEncode(parameters.Ciphertext);
    auto& iv = parameters.GetIv();

    if (iv.size() > 0)
    {
      payload[IvValue] = Base64Url::Base64UrlEncode(iv);
    }

    if (parameters.AdditionalAuthenticatedData.size() > 0)
    {
      payload[AdditionalAuthenticatedValue]
          = Base64Url::Base64UrlEncode(parameters.AdditionalAuthenticatedData);
    }

    if (parameters.AuthenticationTag.size() > 0)
    {
      payload[TagsPropertyName] = Base64Url::Base64UrlEncode(parameters.AuthenticationTag);
    }

    return payload.dump();
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

#include <azure/core/base64.hpp>

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

    if (!iv.empty())
    {
      payload[IvValue] = Base64Url::Base64UrlEncode(iv);
    }

    if (!parameters.AdditionalAuthenticatedData.empty())
    {
      payload[AdditionalAuthenticatedValue]
          = Base64Url::Base64UrlEncode(parameters.AdditionalAuthenticatedData);
    }

    if (!parameters.AuthenticationTag.empty())
    {
      payload[TagsPropertyName] = Base64Url::Base64UrlEncode(parameters.AuthenticationTag);
    }

    return payload.dump();
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "../private/key_verify_parameters.hpp"

#include "../private/cryptography_serializers.hpp"
#include "../private/key_constants.hpp"

#include <azure/core/base64.hpp>

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    std::string KeyVerifyParametersSerializer::KeyVerifyParametersSerialize(
        KeyVerifyParameters const& parameters)
    {
      Azure::Core::Json::_internal::json payload;

      using namespace Azure::Security::KeyVault::Keys::_detail;
      using namespace Azure::Core::_internal;
      payload[AlgorithmValue] = parameters.Algorithm;
      payload[ValueParameterValue] = Base64Url::Base64UrlEncode(parameters.Signature);
      payload[DigestValue] = Base64Url::Base64UrlEncode(parameters.Digest);

      return payload.dump();
    }
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

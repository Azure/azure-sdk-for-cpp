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
  namespace _detail {

    std::string KeyWrapParametersSerializer::KeyWrapParametersSerialize(
        KeyWrapParameters const& parameters)
    {
      Azure::Core::Json::_internal::json payload;

      using namespace Azure::Security::KeyVault::Keys::_detail;
      using namespace Azure::Core::_internal;
      payload[AlgorithmValue] = parameters.Algorithm;
      payload[ValueParameterValue] = Base64Url::Base64UrlEncode(parameters.Key);

      return payload.dump();
    }
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
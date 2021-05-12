// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/cryptography/cryptography_serializers.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <azure/keyvault/common/internal/base64url.hpp>

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
      using namespace Azure::Security::KeyVault::_internal;
      payload[AlgorithmValue] = parameters.Algorithm;
      payload[ValueParameterValue] = Base64Url::Base64UrlEncode(parameters.Key);

      return payload.dump();
    }
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail

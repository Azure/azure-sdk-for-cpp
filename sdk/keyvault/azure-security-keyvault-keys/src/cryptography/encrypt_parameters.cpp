// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_serializers.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <azure/keyvault/common/internal/base64url.hpp>

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  std::string _detail::EncryptParametersSerializer::EncryptParametersSerialize(
      EncryptParameters const& parameters)
  {
    Azure::Core::Json::_internal::json payload;

    using namespace Azure::Security::KeyVault::Keys::_detail;
    using namespace Azure::Security::KeyVault::_internal;
    payload[AlgorithmValue] = parameters.Algorithm.ToString();
    payload[EncryptParameterValue] = Base64Url::Base64UrlEncode(parameters.Plaintext);

    if (parameters.Iv.size() > 0)
    {
      payload[IvValue] = Base64Url::Base64UrlEncode(parameters.Iv);
    }

    if (parameters.AdditionalAuthenticatedData.size() > 0)
    {
      payload[AdditionalAuthenticatedValue]
          = Base64Url::Base64UrlEncode(parameters.AdditionalAuthenticatedData);
    }

    return payload.dump();
  }
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

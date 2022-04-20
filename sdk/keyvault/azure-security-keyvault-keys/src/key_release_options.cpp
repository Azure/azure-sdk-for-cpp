// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_options.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;

std::string
Azure::Security::KeyVault::Keys::_detail::KeyReleaseOptionsSerializer::KeyReleaseOptionsSerialize(
    KeyReleaseOptions const& keyReleaseOptions)
{
  Azure::Core::Json::_internal::json payload;

  payload[_detail::TargetValue] = keyReleaseOptions.Target;
  
  JsonOptional::SetFromNullable<KeyEncryptionAlgorithm, std::string>(
      keyReleaseOptions.Encryption,
      payload,
      _detail::EncryptionValue,
      [](KeyEncryptionAlgorithm enc) { return enc.ToString(); });

  JsonOptional::SetFromNullable(keyReleaseOptions.Nonce, payload, _detail::NonceValue);

  return payload.dump();
}

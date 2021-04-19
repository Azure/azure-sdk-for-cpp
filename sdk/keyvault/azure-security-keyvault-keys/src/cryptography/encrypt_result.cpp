// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_serializers.hpp"

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  EncryptResult _detail::EncryptResultSerializer::EncryptResultDeserialize(
      Azure::Core::Http::RawResponse const&)
  {
    return EncryptResult();
  }

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

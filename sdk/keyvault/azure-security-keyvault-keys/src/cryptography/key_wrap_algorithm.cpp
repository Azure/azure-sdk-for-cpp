// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  const KeyWrapAlgorithm KeyWrapAlgorithm::Rsa15(_detail::Rsa15Value);
  const KeyWrapAlgorithm KeyWrapAlgorithm::RsaOaep(_detail::RsaOaepValue);
  const KeyWrapAlgorithm KeyWrapAlgorithm::RsaOaep256(_detail::RsaOaep256Value);
  const KeyWrapAlgorithm KeyWrapAlgorithm::A128KW(_detail::A128KWValueValue);
  const KeyWrapAlgorithm KeyWrapAlgorithm::A192KW(_detail::A192KWValueValue);
  const KeyWrapAlgorithm KeyWrapAlgorithm::A256KW(_detail::A256KWValueValue);
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
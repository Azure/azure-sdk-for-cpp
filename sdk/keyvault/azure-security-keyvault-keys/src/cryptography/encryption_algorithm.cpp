// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../private/key_constants.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  const EncryptionAlgorithm EncryptionAlgorithm::Rsa15(_detail::Rsa15Value);
  const EncryptionAlgorithm EncryptionAlgorithm::RsaOaep(_detail::RsaOaepValue);
  const EncryptionAlgorithm EncryptionAlgorithm::RsaOaep256(_detail::RsaOaep256Value);
  const EncryptionAlgorithm EncryptionAlgorithm::A128Gcm(_detail::A128GcmValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A192Gcm(_detail::A192GcmValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A256Gcm(_detail::A256GcmValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A128Cbc(_detail::A128CbcValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A192Cbc(_detail::A192CbcValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A256Cbc(_detail::A256CbcValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A128CbcPad(_detail::A128CbcPadValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A192CbcPad(_detail::A192CbcPadValue);
  const EncryptionAlgorithm EncryptionAlgorithm::A256CbcPad(_detail::A256CbcPadValue);
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

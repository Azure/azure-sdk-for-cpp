// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::CkmRsaAesKeyWrap(
      _detail::CKM_RSA_AES_KEY_WRAP_Value);

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RsaAesKeyWrap256(
      _detail::RSA_AES_KEY_WRAP_256_Value);

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RsaAesKeyWrap384(
      _detail::RSA_AES_KEY_WRAP_384_Value);
}}}} // namespace Azure::Security::KeyVault::Keys

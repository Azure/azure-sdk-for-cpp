// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::CKM_RSA_AES_KEY_WRAP(
      _detail::CKM_RSA_AES_KEY_WRAP_Value);

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_256(
      _detail::RSA_AES_KEY_WRAP_256_Value);

  const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_384(
      _detail::RSA_AES_KEY_WRAP_384_Value);
}}}} // namespace Azure::Security::KeyVault::Keys

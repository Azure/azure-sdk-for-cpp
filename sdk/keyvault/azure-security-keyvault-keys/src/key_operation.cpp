//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

using namespace Azure::Security::KeyVault::Keys::_detail;

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyOperation KeyOperation::Encrypt(EncryptValue);

  const KeyOperation KeyOperation::Decrypt(DecryptValue);

  const KeyOperation KeyOperation::Sign(SignValue);

  const KeyOperation KeyOperation::Verify(VerifyValue);

  const KeyOperation KeyOperation::WrapKey(WrapKeyValue);

  const KeyOperation KeyOperation::UnwrapKey(UnwrapKeyValue);

  const KeyOperation KeyOperation::Import(ImportValue);

  const KeyOperation KeyOperation::Export(ExportValue);
}}}} // namespace Azure::Security::KeyVault::Keys

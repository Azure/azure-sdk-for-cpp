// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_operation_type.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyOperationType KeyOperationType::Encrypt("encrypt");

  const KeyOperationType KeyOperationType::Decrypt("decrypt");

  const KeyOperationType KeyOperationType::Sign("sign");

  const KeyOperationType KeyOperationType::Verify("verify");

  const KeyOperationType KeyOperationType::WrapKey("wrapKey");

  const KeyOperationType KeyOperationType::UnwrapKey("unwrapKey");

  const KeyOperationType KeyOperationType::Import("import");

}}}} // namespace Azure::Security::KeyVault::Keys

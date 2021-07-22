// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_client_models.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyOperation KeyOperation::Encrypt("encrypt");

  const KeyOperation KeyOperation::Decrypt("decrypt");

  const KeyOperation KeyOperation::Sign("sign");

  const KeyOperation KeyOperation::Verify("verify");

  const KeyOperation KeyOperation::WrapKey("wrapKey");

  const KeyOperation KeyOperation::UnwrapKey("unwrapKey");

  const KeyOperation KeyOperation::Import("import");

}}}} // namespace Azure::Security::KeyVault::Keys

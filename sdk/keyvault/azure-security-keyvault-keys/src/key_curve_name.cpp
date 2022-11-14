//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  const KeyCurveName KeyCurveName::P256(_detail::P256Value);

  const KeyCurveName KeyCurveName::P256K(_detail::P256KValue);

  const KeyCurveName KeyCurveName::P384(_detail::P384Value);

  const KeyCurveName KeyCurveName::P521(_detail::P521Value);

}}}} // namespace Azure::Security::KeyVault::Keys

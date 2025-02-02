// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

using namespace Azure::Security::KeyVault::Keys;

const KeyVaultKeyType KeyVaultKeyType::Ec(_detail::EcValue);
const KeyVaultKeyType KeyVaultKeyType::EcHsm(_detail::EcHsmValue);
const KeyVaultKeyType KeyVaultKeyType::Rsa(_detail::RsaValue);
const KeyVaultKeyType KeyVaultKeyType::RsaHsm(_detail::RsaHsmValue);
const KeyVaultKeyType KeyVaultKeyType::Oct(_detail::OctValue);
const KeyVaultKeyType KeyVaultKeyType::OctHsm(_detail::OctHsmValue);

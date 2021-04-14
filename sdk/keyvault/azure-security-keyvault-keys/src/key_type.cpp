// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <stdexcept>

using namespace Azure::Security::KeyVault::Keys;

const JsonWebKeyType JsonWebKeyType::Ec(_detail::EcValue);
const JsonWebKeyType JsonWebKeyType::EcHsm(_detail::EcHsmValue);
const JsonWebKeyType JsonWebKeyType::Rsa(_detail::RsaValue);
const JsonWebKeyType JsonWebKeyType::RsaHsm(_detail::RsaHsmValue);
const JsonWebKeyType JsonWebKeyType::Oct(_detail::OctValue);
const JsonWebKeyType JsonWebKeyType::OctHsm(_detail::OctHsmValue);

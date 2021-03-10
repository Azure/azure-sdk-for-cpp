// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

using namespace Azure::Security::KeyVault::Keys;

KeyCurveName KeyCurveName::P256() { return KeyCurveName(_detail::P256Value); }

KeyCurveName KeyCurveName::P256K() { return KeyCurveName(_detail::P256KValue); }

KeyCurveName KeyCurveName::P384() { return KeyCurveName(_detail::P384Value); }

KeyCurveName KeyCurveName::P521() { return KeyCurveName(_detail::P521Value); }

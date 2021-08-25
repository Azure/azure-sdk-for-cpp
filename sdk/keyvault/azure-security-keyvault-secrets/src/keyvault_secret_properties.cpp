// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Declares SecretProperties.
 *
 */

#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include "private/secret_serializers.hpp"

using namespace Azure::Security::KeyVault::Secrets;

SecretProperties::SecretProperties(Azure::Core::Url const& url)
{
  _detail::SecretSerializer::ParseIDUrl(*this, url.GetAbsoluteUrl());
}

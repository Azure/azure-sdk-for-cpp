// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Secret Client.
 *
 */

#pragma once

#include <cstddef>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {

  constexpr static const char KeyVaultServicePackageName[] = "keyvault-secrets";

  /**************** KeyVault QueryParameters *********/
  static constexpr char const ApiVersion[] = "api-version";
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail

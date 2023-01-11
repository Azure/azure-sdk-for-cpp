// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Options that allow you to configure the #CryptographyClient for local or remote operations
 * on Key Vault.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/keys/dll_import_export.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Options that allow you to configure the #CryptographyClient for local or remote
   * operations on Key Vault.
   *
   */
  struct CryptographyClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Gets the #ServiceVersion of the service API used when making requests. For more
     * information, see [Key Vault
     * versions](https://docs.microsoft.com/rest/api/keyvault/key-vault-versions).
     *
     */
    std::string Version;

    /**
     * @brief Construct a new Key Client Options object.
     *
     */
    CryptographyClientOptions() : Azure::Core::_internal::ClientOptions()
    {
      Version = "7.4-preview.1";
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

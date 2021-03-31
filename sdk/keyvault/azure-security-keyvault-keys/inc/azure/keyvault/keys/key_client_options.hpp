// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Keys client.
 *
 */

#pragma once

#include <azure/keyvault/common/client_options.hpp>

#include "azure/keyvault/keys/key_vault_key.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct KeyClientOptions : public Azure::Security::KeyVault::Common::ClientOptions
  {
    /**
     * @brief Construct a new Key Client Options object.
     *
     * @param version Optional version for the client.
     */
    KeyClientOptions(
        Azure::Security::KeyVault::Common::ServiceVersion version
        = Azure::Security::KeyVault::Common::ServiceVersion::V7_2)
        : ClientOptions(version)
    {
    }
  };

}}}} // namespace Azure::Security::KeyVault::Keys

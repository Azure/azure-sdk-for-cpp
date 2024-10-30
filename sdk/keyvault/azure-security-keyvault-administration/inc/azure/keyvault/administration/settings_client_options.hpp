// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Administration client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>
#include <azure/keyvault/administration/dll_import_export.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {

  /**
   * @brief Define the options to create a Key Vault Administration client.
   *
   */
  struct SettingsClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Service Version used.
     *
     */
    std::string ApiVersion{"7.4"};
  };

}}}} // namespace Azure::Security::KeyVault::Administration

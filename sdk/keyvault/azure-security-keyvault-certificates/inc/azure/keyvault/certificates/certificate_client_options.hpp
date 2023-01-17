// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Certificate client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/dll_import_export.hpp"
#include <memory>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  /**
   * @brief Define the options to create an SDK Certificate client.
   *
   */
  struct CertificateClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Service Version used.
     *
     */
    const std::string ApiVersion{"7.3"};
  };

}}}} // namespace Azure::Security::KeyVault::Certificates

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Certificate client.
 *
 */

#pragma once

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/dll_import_export.hpp"

#include <azure/core/internal/client_options.hpp>

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
    std::string ApiVersion{"7.6-preview.2"};
  };

}}}} // namespace Azure::Security::KeyVault::Certificates

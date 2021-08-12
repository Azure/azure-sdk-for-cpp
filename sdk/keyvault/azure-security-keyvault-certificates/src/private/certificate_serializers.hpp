// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/certificate_client_options.hpp"

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {
    /***************** Certificate  *****************/
    class KeyVaultCertificateSerializer final {
    public:
      // Creates a new key based on a name and an HTTP raw response.
      static KeyVaultCertificateWithPolicy KeyVaultCertificateDeserialize(
          std::string const& name,
          Azure::Core::Http::RawResponse const& rawResponse);
    };

}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

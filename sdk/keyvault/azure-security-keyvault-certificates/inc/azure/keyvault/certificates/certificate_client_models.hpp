// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey types.
 *
 */

#pragma once

#include "azure/keyvault/certificates/dll_import_export.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  struct KeyVaultCertificateWithPolicy
  {
  };
}}}} // namespace Azure::Security::KeyVault::Certificates

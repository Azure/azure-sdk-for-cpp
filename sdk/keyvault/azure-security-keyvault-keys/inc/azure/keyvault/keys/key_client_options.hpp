// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the supported options to create a Key Vault Keys client.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>

#include "azure/keyvault/keys/key_vault_key.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  enum class ServiceVersion
  {
    V7_0,
    V7_1,
    V7_2
  };

  struct KeyClientOptions
  {
    ServiceVersion Version;
    Azure::Core::Http::RetryOptions RetryOptions;
    Azure::Core::Http::TransportPolicyOptions TransportPolicyOptions;
    Azure::Core::Http::TelemetryPolicyOptions TelemetryPolicyOptions;

    KeyClientOptions(ServiceVersion version = ServiceVersion::V7_2) : Version(version) {}

    std::string GetVersionString()
    {
      switch (Version)
      {
        case ServiceVersion::V7_0:
          return "7.0";
        case ServiceVersion::V7_1:
          return "7.1";
        case ServiceVersion::V7_2:
          return "7.2";
        default:
          throw std::runtime_error("Version not found");
      }
    }
  };

}}}} // namespace Azure::Security::KeyVault::Keys

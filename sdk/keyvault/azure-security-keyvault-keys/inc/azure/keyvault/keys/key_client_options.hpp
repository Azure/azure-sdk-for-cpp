// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>
#include <azure/keyvault/common/keyvault_pipeline.hpp>

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

    KeyClientOptions(ServiceVersion version = ServiceVersion::V7_1) : Version(version) {}

    std::string GetVersionString()
    {
      switch (Version)
      {
        case ServiceVersion::V7_0:
          return "7.0";
        case ServiceVersion::V7_1:
          return "7.1";
        default:
          throw std::runtime_error("Version not found");
      }
    }
  };

}}}} // namespace Azure::Security::KeyVault::Keys

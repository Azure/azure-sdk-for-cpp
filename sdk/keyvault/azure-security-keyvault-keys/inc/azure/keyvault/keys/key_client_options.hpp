// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Keys client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/keys/key_vault_key.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  enum class ServiceVersion
  {
    /**
     * @brief Use to send request to the 7.0 version of Key Vault service.
     *
     */
    V7_0,
    /**
     * @brief Use to send request to the 7.1 version of Key Vault service.
     *
     */
    V7_1,
    /**
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    V7_2
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct KeyClientOptions : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;

    /**
     * @brief Construct a new Key Client Options object.
     *
     * @param version Optional version for the client.
     */
    KeyClientOptions(ServiceVersion version = ServiceVersion::V7_2)
        : Azure::Core::_internal::ClientOptions(), Version(version)
    {
    }

    std::string GetVersionString() const
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the base options to create a Key Vault client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include <stdexcept>
#include <string>

namespace Azure { namespace Security { namespace KeyVault {

  /**
   * @brief Available and supported service versions.
   *
   */
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
   * @brief Define the base options to create an KeyVault SDK client.
   *
   */
  struct ClientOptions : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief The service version. All request are created with this version.
     *
     */
    ServiceVersion Version;

    ClientOptions(ServiceVersion version)
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

}}} // namespace Azure::Security::KeyVault

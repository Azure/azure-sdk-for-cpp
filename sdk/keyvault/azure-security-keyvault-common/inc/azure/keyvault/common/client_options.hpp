// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the base options to create a Key Vault client.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/internal/client_options.hpp>
#include <azure/core/response.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Common {

  /**
   * @brief Available and supported service versions.
   *
   */
  enum class ServiceVersion
  {
    V7_0,
    V7_1,
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

}}}} // namespace Azure::Security::KeyVault::Common

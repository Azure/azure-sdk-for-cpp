// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Key Vault Keys client.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/keys/dll_import_export.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  class ServiceVersion {
  private:
    std::string m_version;

  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for the Key Vault keys service.
     */
    ServiceVersion(std::string version) : m_version(std::move(version)) {}

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(ServiceVersion const& other) const { return m_version == other.m_version; }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_version; }

    /**
     * @brief Use to send request to the 7.0 version of Key Vault service.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const ServiceVersion V7_0;

    /**
     * @brief Use to send request to the 7.1 version of Key Vault service.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const ServiceVersion V7_1;

    /**
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const ServiceVersion V7_2;
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
  };
}}}} // namespace Azure::Security::KeyVault::Keys

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
   * @brief The API version to use from Key Vault.
   *
   */
  class ServiceVersion final {
  private:
    std::string m_version;

  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for the Key Vault Certificate service.
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
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const ServiceVersion V7_3;
  };

  /**
   * @brief Define the options to create an SDK Certificate client.
   *
   */
  struct CertificateClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;

    /**
     * @brief Construct a new Certificate Client Options object.
     *
     * @param version Optional version for the client.
     */
    CertificateClientOptions(ServiceVersion version = ServiceVersion::V7_3)
        : Azure::Core::_internal::ClientOptions(), Version(version)
    {
    }
  };

}}}} // namespace Azure::Security::KeyVault::Certificates

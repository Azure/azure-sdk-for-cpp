// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Options that allow you to configure the #CryptographyClient for local or remote operations
 * on Key Vault.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/keyvault/keys/dll_import_export.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  /**
   * @brief Represent the Key Vault Keys Service Version.
   *
   */
  class ServiceVersion final {
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
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const ServiceVersion V7_2;
  };

  /**
   * @brief Options that allow you to configure the #CryptographyClient for local or remote
   * operations on Key Vault.
   *
   */
  struct CryptographyClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Gets the #ServiceVersion of the service API used when making requests. For more
     * information, see [Key Vault
     * versions](https://docs.microsoft.com/rest/api/keyvault/key-vault-versions).
     *
     */
    ServiceVersion Version;

    /**
     * @brief Construct a new Key Client Options object.
     *
     * @param version Optional version for the client.
     */
    CryptographyClientOptions(ServiceVersion version = ServiceVersion::V7_2)
        : Azure::Core::_internal::ClientOptions(), Version(version)
    {
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography

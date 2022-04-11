// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret actions options
 */
#pragma once
#include "dll_import_export.hpp"
#include <azure/core/internal/client_options.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

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
    AZURE_SECURITY_KEYVAULT_SECRETS_DLLEXPORT static const ServiceVersion V7_2;

    /**
     * @brief Use to send request to the 7.2 version of Key Vault service.
     *
     */
    AZURE_SECURITY_KEYVAULT_SECRETS_DLLEXPORT static const ServiceVersion V7_3;
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct SecretClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Service Version used.
     *
     */
    const ServiceVersion Version;

    /**
     * @brief Construct a new Secret Client Options object.
     *
     * @param version Optional version for the client.
     */
    SecretClientOptions(ServiceVersion version = ServiceVersion::V7_3)
        : Azure::Core::_internal::ClientOptions(), Version(version)
    {
    }
  };

  /**
   * @brief Optional parameters for SecretClient::GetSecret
   *
   */
  struct GetSecretOptions final
  {
    /**
     * @brief Specify the secret version to get.
     *
     */
    std::string Version;
  };

  /**
   * @brief Optional parameters for SecretClient::UpdateSecretParameters
   *
   */
  struct UpdateSecretPropertiesOptions final
  {
    /**
     * @brief Specify the secret version to update.
     *
     */
    std::string Version;
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfSecrets.
   *
   */
  struct GetPropertiesOfSecretsOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfSecretVersions.
   *
   */
  struct GetPropertiesOfSecretVersionsOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };

  /**
   * @brief The options for calling an operation #GetDeletedSecrets.
   *
   */
  struct GetDeletedSecretsOptions final
  {
    Azure::Nullable<std::string> NextPageToken;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

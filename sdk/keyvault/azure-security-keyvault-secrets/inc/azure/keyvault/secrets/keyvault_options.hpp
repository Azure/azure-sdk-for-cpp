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
    const std::string Version;

    /**
     * @brief Construct a new Secret Client Options object.
     *
     * @param version Optional version for the client.
     */
    SecretClientOptions(std::string version = "7.3")
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

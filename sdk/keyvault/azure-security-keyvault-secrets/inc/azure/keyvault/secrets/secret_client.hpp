// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines the Key Vault Secret client.
 *
 */
#pragma once
#include "dll_import_export.hpp"
#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
#include <string>

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
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct SecretClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;

    /**
     * @brief Construct a new Secret Client Options object.
     *
     * @param version Optional version for the client.
     */
    SecretClientOptions(ServiceVersion version = ServiceVersion::V7_2)
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
     * @brief Specify the key version to get.
     *
     */
    std::string Version;
  };

  /**
   * @brief The SecretClient provides synchronous methods to manage a secret in the Azure Key
   * Vault. The client supports creating, retrieving, updating, deleting, purging, backing up,
   * restoring, and listing the secret.
   */
  class SecretClient
#if !defined(TESTING_BUILD)
      final
#endif
  {

  protected:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> m_pipeline;

  public:
    /**
     * @brief Construct a new SecretClient object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit SecretClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        SecretClientOptions options = SecretClientOptions());

    /**
     * @brief Construct a new Key Client object from another key client.
     *
     * @param keyClient An existing key vault key client.
     */
    explicit SecretClient(SecretClient const& keyClient) : m_pipeline(keyClient.m_pipeline){};

    ~SecretClient() = default;

    /**
     * @brief Get a specified secret from a given key vault
     * ThisT operation is applicable to any secret stored in Azure Key Vault.
     * This operation requires the secrets/get permission.
     *
     * @param name The name of the secret
     * @param options The optional parameters for this request.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    Azure::Response<KeyVaultSecret> GetSecret(
        std::string const& name,
        GetSecretOptions const& options = GetSecretOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Sets a secret in a specified key vault.
     * This operation adds a secret to the Azure Key Vault. 
     * If the named secret already exists, Azure Key Vault creates a new version of that secret. 
     * This operation requires the secrets/set permission.
     *
     * @param name The name of the secret
     * @param value The the value of the secret.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    /* Azure::Response<KeyVaultSecret> SetSecret(
        std::string const& name,
        std::string const& value,
        Azure::Core::Context const& context = Azure::Core::Context()) const;
        */
    /**
     * @brief Sets a secret in a specified key vault.
     * This operation adds a secret to the Azure Key Vault.
     * If the named secret already exists, Azure Key Vault creates a new version of that secret.
     * This operation requires the secrets/set permission.
     *
     * @param secret The secret to set.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The Secret wrapped in the Response.
     */
    /* Azure::Response<KeyVaultSecret> SetSecret(
        KeyVaultSecret const& secret,
        Azure::Core::Context const& context = Azure::Core::Context()) const;*/

    std::string ClientVersion() const;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Keyvault Secret definition
 */

#pragma once
#include "azure/keyvault/secrets/keyvault_secret_properties.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  class SecretClient;
  class SecretPropertiesPagedResponse;
  namespace _detail { namespace Models {
    struct SecretBundle;
    struct SecretItem;
    struct SecretSetParameters;
  }} // namespace _detail::Models
  /**
   * @brief Secret is the resource consisting of name, value and its attributes specified in
   * SecretProperties. It is managed by Secret Service.
   *
   */
  struct KeyVaultSecret
  {
    /**
     * @brief The name of the secret.
     *
     */
    std::string Name;

    /**
     * @brief The secret value.
     *
     */
    Azure::Nullable<std::string> Value;

    /**
     * @brief The secret id.
     *
     */
    std::string Id;

    /**
     * @brief The secret Properties bundle.
     *
     */
    SecretProperties Properties;

    /**
     * @brief Construct a new Secret object.
     *
     */
    KeyVaultSecret() = default;

    /**
     * @brief Construct a new Secret object.
     *
     * @param name The name of the secret.
     * @param value The name of the secret.
     */
    KeyVaultSecret(std::string const& name, std::string const& value);

  private:
    KeyVaultSecret(std::string name);
    KeyVaultSecret(_detail::Models::SecretBundle const& secret);
    KeyVaultSecret(_detail::Models::SecretItem const& secret);
    _detail::Models::SecretSetParameters ToSetSecretParameters() const;

    friend struct DeletedSecret;
    friend class SecretClient;
    friend class SecretPropertiesPagedResponse;
  };

}}}} // namespace Azure::Security::KeyVault::Secrets

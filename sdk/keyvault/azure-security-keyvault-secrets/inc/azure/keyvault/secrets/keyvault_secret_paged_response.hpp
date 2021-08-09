// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines the Key Vault Secret paged responses.
 *
 */

#pragma once

#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret_properties.hpp"
#include <azure/core/paged_response.hpp>
#include <memory>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  // forward definition
  class SecretClient;

  /**
   * @brief Define a single page to list the secrets from the Key Vault.
   *
   */
  class KeyVaultSecretPropertiesPagedResponse final
      : public Azure::Core::PagedResponse<KeyvaultSecretProperties> {
  private:
    friend class SecretClient;
    friend class Azure::Core::PagedResponse<KeyvaultSecretProperties>;

    std::string m_secretName;
    std::shared_ptr<SecretClient> m_secretClient;

    void OnNextPage(const Azure::Core::Context& context);

    KeyVaultSecretPropertiesPagedResponse(
        KeyVaultSecretPropertiesPagedResponse&& secretProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient,
        std::string const& secretName = std::string())
        : PagedResponse(std::move(secretProperties)), m_secretName(secretName),
          m_secretClient(secretClient), Items(std::move(secretProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new KeyVaultSecretPropertiesPagedResponse object.
     *
     */
    KeyVaultSecretPropertiesPagedResponse() = default;

    /**
     * @brief Each #KeyvaultSecretProperties represent a Secret in the Key Vault.
     *
     */
    std::vector<KeyvaultSecretProperties> Items;
  };

  /**
   * @brief Define a single page containing the deleted keys from the Key Vault.
   *
   */
  class KeyvaultSecretDeletedSecretPagedResponse final
      : public Azure::Core::PagedResponse<KeyvaultSecretDeletedSecretPagedResponse> {
  private:
    friend class SecretClient;
    friend class Azure::Core::PagedResponse<KeyvaultSecretDeletedSecretPagedResponse>;

    std::shared_ptr<SecretClient> m_secretClient;
    void OnNextPage(const Azure::Core::Context& context);

    KeyvaultSecretDeletedSecretPagedResponse(
        KeyvaultSecretDeletedSecretPagedResponse&& deletedKeyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient)
        : PagedResponse(std::move(deletedKeyProperties)), m_secretClient(secretClient),
          Items(std::move(deletedKeyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new Deleted Key Single Page object
     *
     */
    KeyvaultSecretDeletedSecretPagedResponse() = default;

    /**
     * @brief Each #DeletedKey represent a deleted key in the Key Vault.
     *
     */
    std::vector<KeyVaultDeletedSecret> Items;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

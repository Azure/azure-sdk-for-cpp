// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines the Key Vault Secret paged responses.
 *
 */

#pragma once

#include "azure/keyvault/secrets/generated/key_vault_client_paged_responses.hpp"
#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret_properties.hpp"

#include <azure/core/paged_response.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  // forward definition
  class SecretClient;

  /**
   * @brief Define a single page to list the secrets from the Key Vault.
   *
   */
  class SecretPropertiesPagedResponse final
      : public Azure::Core::PagedResponse<SecretPropertiesPagedResponse> {
  private:
    friend class SecretClient;
    friend class Azure::Core::PagedResponse<SecretPropertiesPagedResponse>;

    std::string m_secretName;
    std::shared_ptr<SecretClient> m_secretClient;
    Generated::GetSecretsPagedResponse m_generatedResponse;
    void OnNextPage(const Azure::Core::Context& context);

    SecretPropertiesPagedResponse(
        SecretPropertiesPagedResponse&& secretProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient,
        std::string const& secretName = std::string())
        : PagedResponse(std::move(secretProperties)), m_secretName(secretName),
          m_secretClient(std::move(secretClient)), Items(std::move(secretProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

    SecretPropertiesPagedResponse(
        Generated::GetSecretsPagedResponse& secretPagedResponse,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient,
        std::string const& secretName = std::string())
        : m_secretName(secretName), m_secretClient(std::move(secretClient)),
          m_generatedResponse(std::move(secretPagedResponse))
    {
      for (auto& item : m_generatedResponse.Value.Value())
      {
        Items.push_back(KeyVaultSecret(item).Properties);
      }
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new SecretPropertiesPagedResponse object.
     *
     */
    SecretPropertiesPagedResponse() = default;

    /**
     * @brief Each #Azure::Security::KeyVault::Secrets::SecretProperties represent a Secret in the
     * Key Vault.
     *
     */
    std::vector<SecretProperties> Items;
  };

  /**
   * @brief Define a single page containing the deleted keys from the Key Vault.
   *
   */
  class DeletedSecretPagedResponse final
      : public Azure::Core::PagedResponse<DeletedSecretPagedResponse> {
  private:
    friend class SecretClient;
    friend class Azure::Core::PagedResponse<DeletedSecretPagedResponse>;

    Generated::GetDeletedSecretsPagedResponse m_generatedResponse;
    std::shared_ptr<SecretClient> m_secretClient;
    void OnNextPage(const Azure::Core::Context& context);

    DeletedSecretPagedResponse(
        DeletedSecretPagedResponse&& deletedKeyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient)
        : PagedResponse(std::move(deletedKeyProperties)), m_secretClient(std::move(secretClient)),
          Items(std::move(deletedKeyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }
    DeletedSecretPagedResponse(
        Generated::GetDeletedSecretsPagedResponse& secretPagedResponse,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<SecretClient> secretClient)
        : m_secretClient(std::move(secretClient)),
          m_generatedResponse(std::move(secretPagedResponse))
    {
      for (auto& item : m_generatedResponse.Value.Value())
      {
        Items.push_back(DeletedSecret(item));
      }
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new Deleted Key Single Page object
     *
     */
    DeletedSecretPagedResponse() = default;

    /**
     * @brief Each #Azure::Security::KeyVault::Secrets::DeletedSecret represent a deleted secret in
     * the Key Vault.
     *
     */
    std::vector<DeletedSecret> Items;
  };
}}}} // namespace Azure::Security::KeyVault::Secrets

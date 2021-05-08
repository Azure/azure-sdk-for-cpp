// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a page of listing keys from a Key Vault.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/paged_response.hpp>

#include <azure/keyvault/common/internal/single_page.hpp>

#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <memory>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {
  class KeyClient;

  /**
   * @brief Define a single page to list the keys from the Key Vault.
   *
   */
  class KeyPropertiesPageResult final : public Azure::Core::PagedResponse<KeyPropertiesPageResult> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<KeyPropertiesPageResult>;

    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param keyProperties A previously created #KeyPropertiesPageResult that is used to init this
     * instance.
     * @param rawResponse The Http raw response from where the #KeyPropertiesPageResult was parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    KeyPropertiesPageResult(
        KeyPropertiesPageResult&& keyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<KeyClient> keyClient,
        std::string const& keyName = std::string())
        : m_keyName(keyName), m_keyClient(keyClient), Items(std::move(keyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new key properties object.
     *
     */
    KeyPropertiesPageResult() = default;

    /**
     * @brief Each #KeyProperties represent a Key in the Key Vault.
     *
     */
    std::vector<KeyProperties> Items;
  };

  /**
   * @brief Define a single page containing the deleted keys from the Key Vault.
   *
   */
  class DeletedKeyPageResult final : public Azure::Core::PagedResponse<DeletedKeyPageResult> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<DeletedKeyPageResult>;

    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context& context);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param deletedKeyProperties A previously created #DeletedKeyPageResult that is used to init
     * this new instance.
     * @param rawResponse The Http raw response from where the #DeletedKeyPageResult was parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    DeletedKeyPageResult(
        DeletedKeyPageResult&& deletedKeyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<KeyClient> keyClient)
        : m_keyClient(keyClient), Items(std::move(deletedKeyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new Deleted Key Single Page object
     *
     */
    DeletedKeyPageResult() = default;

    /**
     * @brief Each #DeletedKey represent a deleted key in the Key Vault.
     *
     */
    std::vector<DeletedKey> Items;
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeys.
   *
   */
  struct GetPropertiesOfKeysOptions final
      : public Azure::Security::KeyVault::_internal::GetPageResultOptions
  {
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeyVersions.
   *
   */
  struct GetPropertiesOfKeyVersionsOptions final
      : public Azure::Security::KeyVault::_internal::GetPageResultOptions
  {
  };

  /**
   * @brief The options for calling an operation #GetDeletedKeys.
   *
   */
  struct GetDeletedKeysOptions final : public Azure::Security::KeyVault::_internal::GetPageResultOptions
  {
  };
}}}} // namespace Azure::Security::KeyVault::Keys

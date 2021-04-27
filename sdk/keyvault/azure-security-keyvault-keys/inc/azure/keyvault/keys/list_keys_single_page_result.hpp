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
  class KeyPropertiesSinglePage : public Azure::Core::PagedResponse<KeyPropertiesSinglePage> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<KeyPropertiesSinglePage>;

    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param keyProperties A previously created #KeyPropertiesSinglePage that is used to init this
     * instance.
     * @param rawResponse The Http raw response from where the #KeyPropertiesSinglePage was parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    KeyPropertiesSinglePage(
        KeyPropertiesSinglePage&& keyProperties,
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
    KeyPropertiesSinglePage() = default;

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
  class DeletedKeySinglePage : public Azure::Core::PagedResponse<DeletedKeySinglePage> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<DeletedKeySinglePage>;

    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context& context);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param deletedKeyProperties A previously created #DeletedKeySinglePage that is used to init
     * this new instance.
     * @param rawResponse The Http raw response from where the #DeletedKeySinglePage was parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    DeletedKeySinglePage(
        DeletedKeySinglePage&& deletedKeyProperties,
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
    DeletedKeySinglePage() = default;

    /**
     * @brief Each #DeletedKey represent a deleted key in the Key Vault.
     *
     */
    std::vector<DeletedKey> Items;
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeysSinglePage.
   *
   */
  struct GetPropertiesOfKeysSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };

  /**
   * @brief The options for calling an operation #GetPropertiesOfKeyVersionsSinglePage.
   *
   */
  struct GetPropertiesOfKeyVersionsSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };

  /**
   * @brief The options for calling an operation #GetDeletedKeysSinglePage.
   *
   */
  struct GetDeletedKeysSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };
}}}} // namespace Azure::Security::KeyVault::Keys

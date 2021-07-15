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

#include "azure/keyvault/keys/key_client_models.hpp"

#include <memory>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {
  class KeyClient;

  /**
   * @brief Define a single page to list the keys from the Key Vault.
   *
   */
  class KeyPropertiesPagedResponse final
      : public Azure::Core::PagedResponse<KeyPropertiesPagedResponse> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<KeyPropertiesPagedResponse>;

    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param keyProperties A previously created #KeyPropertiesPageResponse that is used to init
     * this instance.
     * @param rawResponse The HTTP raw response from where the #KeyPropertiesPagedResponse was
     * parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    KeyPropertiesPagedResponse(
        KeyPropertiesPagedResponse&& keyProperties,
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
    KeyPropertiesPagedResponse() = default;

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
  class DeletedKeyPagedResponse final : public Azure::Core::PagedResponse<DeletedKeyPagedResponse> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<DeletedKeyPagedResponse>;

    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context& context);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param deletedKeyProperties A previously created #DeletedKeyPagedResponse that is used to
     * init this new instance.
     * @param rawResponse The HTTP raw response from where the #DeletedKeyPagedResponse was parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    DeletedKeyPagedResponse(
        DeletedKeyPagedResponse&& deletedKeyProperties,
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
    DeletedKeyPagedResponse() = default;

    /**
     * @brief Each #DeletedKey represent a deleted key in the Key Vault.
     *
     */
    std::vector<DeletedKey> Items;
  };
}}}} // namespace Azure::Security::KeyVault::Keys

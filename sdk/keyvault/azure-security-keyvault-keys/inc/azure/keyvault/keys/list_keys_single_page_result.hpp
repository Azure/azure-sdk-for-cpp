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

    enum class KeyPropertiesType
    {
      Keys,
      Versions
    };
    KeyPropertiesType m_type;
    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&);

  public:
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

    std::shared_ptr<KeyClient> keyClient;
    void OnNextPage(const Azure::Core::Context& context);

  public:
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

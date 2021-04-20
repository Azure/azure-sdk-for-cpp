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

  class KeyPropertiesSinglePage : public Azure::Core::PagedResponse {
  private:
    friend class KeyClient;

    enum class KeyPropertiesType
    {
      Keys,
      Versions
    };
    KeyPropertiesType m_type;
    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&) override;

  public:
    std::vector<KeyProperties> Items;
  };

  class DeletedKeySinglePage : public Azure::Core::PagedResponse {
  private:
    friend class KeyClient;
    std::shared_ptr<KeyClient> keyClient;
    void OnNextPage(const Azure::Core::Context& context) override;

  public:
    std::vector<DeletedKey> Items;
  };

  struct GetPropertiesOfKeysSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };

  struct GetPropertiesOfKeyVersionsSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };

  struct GetDeletedKeysSinglePageOptions
      : public Azure::Security::KeyVault::_internal::GetSinglePageOptions
  {
  };
}}}} // namespace Azure::Security::KeyVault::Keys

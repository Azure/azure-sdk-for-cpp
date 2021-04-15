// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a page of listing keys from a Key Vault.
 *
 */

#pragma once

#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <azure/keyvault/common/internal/single_page.hpp>

#include <azure/core/http/http.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  struct KeyPropertiesSinglePage : public Azure::Security::KeyVault::_internal::SinglePage
  {
    std::vector<KeyProperties> Items;
  };

  struct DeletedKeySinglePage : public Azure::Security::KeyVault::_internal::SinglePage
  {
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

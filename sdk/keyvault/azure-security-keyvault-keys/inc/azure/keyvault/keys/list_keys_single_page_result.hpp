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

#include <azure/core/http/http.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  struct SinglePage
  {
    Azure::Nullable<std::string> ContinuationToken;
  };

  struct KeyPropertiesSinglePage : public SinglePage
  {
    std::vector<KeyProperties> Items;
  };

  struct DeletedKeySinglePage : public SinglePage
  {
    std::vector<DeletedKey> Items;
  };

  struct GetSinglePageOptions
  {
    Azure::Nullable<std::string> ContinuationToken;
    Azure::Nullable<uint32_t> MaxResults;
  };
  struct GetPropertiesOfKeysSinglePageOptions : public GetSinglePageOptions
  {
  };

  struct GetPropertiesOfKeyVersionsSinglePageOptions : public GetSinglePageOptions
  {
  };

  struct GetDeletedKeysSinglePageOptions : public GetSinglePageOptions
  {
  };  
}}}} // namespace Azure::Security::KeyVault::Keys

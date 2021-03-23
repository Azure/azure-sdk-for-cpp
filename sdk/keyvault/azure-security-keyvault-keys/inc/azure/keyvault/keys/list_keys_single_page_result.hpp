// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a page of listing keys from a Key Vault.
 *
 */

#pragma once

#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <azure/core/http/http.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  struct KeyPropertiesSinglePage
  {
    Azure::Nullable<std::string> ContinuationToken;
    std::vector<KeyProperties> Items;
  };

  struct GetSinglePageOptions
  {
    Azure::Nullable<std::string> ContinuationToken;
    Azure::Nullable<uint32_t> MaxResults;
  };
  struct GetPropertiesOfKeysSinglePageOptions : public GetSinglePageOptions
  {
  };

  struct GetPropertiesOfKeyVersionsOptions : public GetSinglePageOptions
  {
  };

  /***********************  Deserializer / Serializer ******************************/
  namespace _detail {
    KeyPropertiesSinglePage KeyPropertiesSinglePageDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);
  } // namespace _detail

}}}} // namespace Azure::Security::KeyVault::Keys

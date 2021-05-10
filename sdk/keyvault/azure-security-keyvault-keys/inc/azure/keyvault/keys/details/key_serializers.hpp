// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/import_key_options.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"
#include "azure/keyvault/keys/list_keys_single_page_result.hpp"

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {
  /***************** KeyVault Key *****************/
  struct KeyVaultKeySerializer final
  {
    // Creates a new key based on a name and an http raw response.
    static KeyVaultKey KeyVaultKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from http raw response only.
    static KeyVaultKey KeyVaultKeyDeserialize(Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a Key based on an Http raw response.
    static void KeyVaultKeyDeserialize(
        KeyVaultKey& key,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from json node directly. Used from listKeys
    static void KeyVaultKeyDeserialize(
        KeyVaultKey& key,
        Azure::Core::Json::_internal::json const& json);

    static std::string GetUrlAuthorityWithScheme(Azure::Core::Url const& url)
    {
      std::string urlString;
      if (!url.GetScheme().empty())
      {
        urlString += url.GetScheme() + "://";
      }
      urlString += url.GetHost();
      if (url.GetPort() != 0)
      {
        urlString += ":" + std::to_string(url.GetPort());
      }
      return urlString;
    }

    void static inline ParseKeyUrl(KeyProperties& keyProperties, std::string const& url)
    {
      Azure::Core::Url kid(url);
      keyProperties.Id = url;
      keyProperties.VaultUrl = GetUrlAuthorityWithScheme(kid);
      auto const& path = kid.GetPath();
      //  path is in the form of `verb/keyName{/keyVersion}`
      auto const separatorChar = '/';
      auto pathEnd = path.end();
      auto start = path.begin();
      start = std::find(start, pathEnd, separatorChar);
      start += 1;
      auto separator = std::find(start, pathEnd, separatorChar);
      if (separator != pathEnd)
      {
        keyProperties.Name = std::string(start, separator);
        start = separator + 1;
        keyProperties.Version = std::string(start, pathEnd);
      }
      else
      {
        // Nothing but the name+
        keyProperties.Name = std::string(start, pathEnd);
      }
    }
  };

  /**************** Deleted Key *******************/
  struct DeletedKeySerializer final
  {
    static DeletedKey DeletedKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  /**************** Import Key Options ***********/
  struct ImportKeyOptionsSerializer final
  {
    static std::string ImportKeyOptionsSerialize(ImportKeyOptions const& importKeyOptions);
  };

  /**************** Key Properties ************/
  struct KeyPropertiesPageResultSerializer final
  {
    static KeyPropertiesPageResult KeyPropertiesPageResultDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);
    static DeletedKeyPageResult DeletedKeyPageResultDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  /**************** JWK  ************/
  struct JsonWebKeySerializer final
  {
    static void JsonWebKeySerialize(
        JsonWebKey const& jwk,
        Azure::Core::Json::_internal::json& destJson);

    static void JsonWebDeserialize(
        JsonWebKey& srcKey,
        Azure::Core::Json::_internal::json const& jsonParser);
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::_detail

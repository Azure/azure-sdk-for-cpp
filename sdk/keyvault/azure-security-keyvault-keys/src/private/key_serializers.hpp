// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/keys/key_client_models.hpp"
#include "azure/keyvault/keys/key_client_options.hpp"

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {
  /***************** KeyVault Key *****************/
  class KeyVaultKeySerializer final {
  public:
    // Creates a new key based on a name and an HTTP raw response.
    static KeyVaultKey KeyVaultKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from HTTP raw response only.
    static KeyVaultKey KeyVaultKeyDeserialize(Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a Key based on an HTTP raw response.
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
  class DeletedKeySerializer final {
  public:
    static DeletedKey DeletedKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  /**************** Import Key Options ***********/
  class ImportKeyOptionsSerializer final {
  public:
    static std::string ImportKeyOptionsSerialize(ImportKeyOptions const& importKeyOptions);
  };

  /**************** Key Properties ************/
  class KeyPropertiesPagedResultSerializer final {
  public:
    static KeyPropertiesPagedResponse KeyPropertiesPagedResultDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);
    static DeletedKeyPagedResponse DeletedKeyPagedResultDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  /**************** JWK  ************/
  class JsonWebKeySerializer final {
  public:
    static void JsonWebKeySerialize(
        JsonWebKey const& jwk,
        Azure::Core::Json::_internal::json& destJson);

    static void JsonWebDeserialize(
        JsonWebKey& srcKey,
        Azure::Core::Json::_internal::json const& jsonParser);
  };

  /**************** KeyReleaseOptionsSerializer  ************/
  class KeyReleaseOptionsSerializer final {
  public:
    static std::string KeyReleaseOptionsSerialize(KeyReleaseOptions const& keyReleaseOptions);
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail

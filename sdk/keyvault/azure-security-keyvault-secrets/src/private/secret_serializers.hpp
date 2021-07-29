// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Serializers/deserializers for the KeyVault Secret client.
 *
 */

#pragma once
#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json.hpp>

using namespace Azure::Security::KeyVault::Secrets;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {
  struct KeyVaultSecretSerializer final
  {
    // Creates a new key based on a name and an HTTP raw response.
    static KeyVaultSecret KeyVaultSecretDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from HTTP raw response only.
    static KeyVaultSecret KeyVaultSecretDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a Key based on an HTTP raw response.
    static void KeyVaultSecretDeserialize(
        KeyVaultSecret& key,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Serializes a key vault secret for set action
    static std::string KeyVaultSecretSerialize(KeyVaultSecret const& parameters);

    // Extract the host out of the URL (with port if available)
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

    // parse the ID url to extract relevant data
    void static inline ParseIDUrl(
        KeyvaultSecretProperties& secretProperties,
        std::string const& url)
    {
      Azure::Core::Url sid(url);
      secretProperties.Id = url;
      secretProperties.VaultUrl = GetUrlAuthorityWithScheme(sid);
      auto const& path = sid.GetPath();
      //  path is in the form of `verb/keyName{/keyVersion}`
      auto const separatorChar = '/';
      auto pathEnd = path.end();
      auto start = path.begin();
      start = std::find(start, pathEnd, separatorChar);
      start += 1;
      auto separator = std::find(start, pathEnd, separatorChar);
      if (separator != pathEnd)
      {
        secretProperties.Name = std::string(start, separator);
        start = separator + 1;
        secretProperties.Version = std::string(start, pathEnd);
      }
      else
      {
        // Nothing but the name+
        secretProperties.Name = std::string(start, pathEnd);
      }
    }
  };

  struct KeyVaultDeletedSecretSerializer final
  {
    // Creates a new deleted secret based on a name and an HTTP raw response.
    static KeyVaultDeletedSecret KeyVaultDeletedSecretDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create deleted secret from HTTP raw response only.
    static KeyVaultDeletedSecret KeyVaultDeletedSecretDeserialize(
        Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a deleted secret based on an HTTP raw response.
    static void KeyVaultDeletedSecretDeserialize(
        KeyVaultDeletedSecret& secret,
        Azure::Core::Http::RawResponse const& rawResponse);
  };

  struct KeyVaultSecretPropertiesSerializer final
  {
    static std::string KeyVaultSecretPropertiesSerialize(
        KeyvaultSecretProperties const& properties);
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail

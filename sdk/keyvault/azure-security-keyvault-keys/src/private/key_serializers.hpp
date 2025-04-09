// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include "azure/keyvault/keys/key_client_models.hpp"
#include "azure/keyvault/keys/key_client_options.hpp"

#include <azure/core/internal/json/json.hpp>

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {
  /***************** KeyVault Key *****************/
  class KeyVaultKeySerializer final {
  public:
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
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail

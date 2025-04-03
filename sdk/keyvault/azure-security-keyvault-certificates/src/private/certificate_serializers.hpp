// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/certificate_client_options.hpp"

#include <azure/core/internal/json/json.hpp>

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {
    /***************** Certificate  *****************/
    class KeyVaultCertificateSerializer final {
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

      void static inline ParseKeyUrl(
          CertificateProperties& certificateProperties,
          std::string const& url)
      {
        Azure::Core::Url kid(url);
        certificateProperties.IdUrl = url;
        certificateProperties.VaultUrl = GetUrlAuthorityWithScheme(kid);
        auto const& path = kid.GetPath();
        //  path is in the form of `verb/keyName{/keyVersion}`
        auto const separatorChar = '/';
        if (path.length() > 0)
        {
          auto pathEnd = path.end();
          auto start = path.begin();
          start = std::find(start, pathEnd, separatorChar);
          start += 1;
          auto separator = std::find(start, pathEnd, separatorChar);
          if (separator != pathEnd)
          {
            certificateProperties.Name = std::string(start, separator);
            start = separator + 1;
            certificateProperties.Version = std::string(start, pathEnd);
          }
          else
          {
            // Nothing but the name+
            certificateProperties.Name = std::string(start, pathEnd);
          }
        }
      }
    };
}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys models.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/certificate_client_options.hpp"

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {
    /***************** Certificate  *****************/
    class KeyVaultCertificateSerializer final {
    public:
      // Creates a new key based on a name and an HTTP raw response.
      static KeyVaultCertificateWithPolicy KeyVaultCertificateDeserialize(
          std::string const& name,
          Azure::Core::Http::RawResponse const& rawResponse);

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
        certificateProperties.Id = url;
        certificateProperties.VaultUrl = GetUrlAuthorityWithScheme(kid);
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
    };

}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

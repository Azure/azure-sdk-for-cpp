// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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
      // Creates a new key based on a name and an HTTP raw response.
      static KeyVaultCertificateWithPolicy Deserialize(
          std::string const& name,
          Azure::Core::Http::RawResponse const& rawResponse);

      static void Deserialize(
          KeyVaultCertificateWithPolicy& certificate,
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

    class CertificatePropertiesSerializer final {
      CertificatePropertiesSerializer() = delete;

    public:
      static std::string Serialize(CertificateProperties const& properties);
      static Azure::Core::Json::_internal::json JsonSerialize(
          CertificateProperties const& properties);
      static void Deserialize(
          CertificateProperties& properties,
          Azure::Core::Json::_internal::json fragment);
    };

    class CertificatePolicySerializer final {
      CertificatePolicySerializer() = delete;

    public:
      static std::string Serialize(CertificatePolicy const& policy);
      static Azure::Core::Json::_internal::json JsonSerialize(CertificatePolicy const& policy);
      static void Deserialize(
          CertificatePolicy& policy,
          Azure::Core::Json::_internal::json fragment);
    };

    class CertificateCreateParametersSerializer final {
      CertificateCreateParametersSerializer() = delete;

    public:
      static std::string Serialize(CertificateCreateParameters const& parameters);
    };

    class CertificateIssuerSerializer final {
      CertificateIssuerSerializer() = delete;

    public:
      static CertificateIssuer Deserialize(
          std::string const& name,
          Azure::Core::Http::RawResponse const& rawResponse);

      static std::string Serialize(CertificateIssuer const& issuer);
    };

    class CertificateContactsSerializer final {
      CertificateContactsSerializer() = delete;

    public:
      static std::string Serialize(std::vector<CertificateContact> const& constacts);
      static std::vector<CertificateContact> Deserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    class CertificateOperationSerializer final {
      CertificateOperationSerializer() = delete;

    public:
      static CertificateOperationProperties Deserialize(
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
          CertificateOperationProperties& certificateProperties,
          std::string const& url)
      {
        Azure::Core::Url kid(url);
        certificateProperties.Id = url;
        certificateProperties.VaultUrl = GetUrlAuthorityWithScheme(kid);
        auto const& path = kid.GetPath();
        // path in format certificates/{name}/pending
        auto const separatorChar = '/';
        auto pathEnd = path.end();
        auto start = path.begin();
        start = std::find(start, pathEnd, separatorChar);
        start += 1;
        auto separator = std::find(start, pathEnd, separatorChar);
        if (separator != pathEnd)
        {
          certificateProperties.Name = std::string(start, separator);
        }
        else
        {
          // Nothing but the name+
          certificateProperties.Name = std::string(start, pathEnd);
        }
      }
    };

    struct DeletedCertificateSerializer final
    {
      static DeletedCertificate Deserialize(
          std::string const& name,
          Azure::Core::Http::RawResponse const& rawResponse);
    };
}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

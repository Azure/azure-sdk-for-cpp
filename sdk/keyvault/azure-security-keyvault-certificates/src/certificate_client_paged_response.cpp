// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/keyvault/certificates/certificate_client.hpp"
#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  void CertificatePropertiesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    // Notes
    // - Before calling `OnNextPage` pagedResponse validates there is a next page, so we are
    // sure NextPageToken is valid.
    // - CertificatePropertiesPagedResponse is used to list certificates from a Key Vault and also
    // to list the key versions from a specific key. When CertificatePropertiesPagedResponse is
    // listing certificates, the `m_certificateName` fields will be empty, but for listing the
    // certificate versions, the CertificatePropertiesPagedResponse needs to keep the name of the
    // key in `m_CertificateName` because it is required to get more pages.
    //
    if (m_certificateName.empty())
    {
      GetPropertiesOfCertificatesOptions options;
      options.NextPageToken = NextPageToken;
      *this = m_certificateClient->GetPropertiesOfCertificates(options, context);
      CurrentPageToken = options.NextPageToken.Value();
    }
    else
    {
      GetPropertiesOfCertificateVersionsOptions options;
      options.NextPageToken = NextPageToken;
      *this = m_certificateClient->GetPropertiesOfCertificateVersions(
          m_certificateName, options, context);
      CurrentPageToken = options.NextPageToken.Value();
    }
  }

  void IssuerPropertiesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    GetPropertiesOfIssuersOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_certificateClient->GetPropertiesOfIssuers(options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }

  void DeletedCertificatesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    GetDeletedCertificatesOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_certificateClient->GetDeletedCertificates(options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }

}}}} // namespace Azure::Security::KeyVault::Certificates
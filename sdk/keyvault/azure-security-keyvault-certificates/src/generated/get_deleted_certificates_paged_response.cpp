// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) TypeSpec Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#include "key_vault_client.hpp"
#include "key_vault_client_paged_responses.hpp"

using namespace Azure::Security::KeyVault::Certificates::_detail;

void GetDeletedCertificatesPagedResponse::OnNextPage(Core::Context const& context)
{
  const auto pageToken = this->NextPageToken;
  this->m_options.NextPageToken = pageToken.Value();
  *this = this->m_client->GetDeletedCertificates(this->m_options, context);
  this->CurrentPageToken = pageToken.Value();
}

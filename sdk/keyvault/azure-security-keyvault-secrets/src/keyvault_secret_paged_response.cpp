// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Defines SecretPropertiesPagedResponse.
 *
 */

#include "azure/keyvault/secrets/keyvault_secret_paged_response.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;

void SecretPropertiesPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  // Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  if (m_secretName.empty())
  {
    GetPropertiesOfSecretsOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_secretClient->GetPropertiesOfSecrets(options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
  else
  {
    GetPropertiesOfSecretVersionsOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_secretClient->GetPropertiesOfSecretsVersions(m_secretName, options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
}

void DeletedSecretPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  // Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  GetDeletedSecretsOptions options;
  options.NextPageToken = NextPageToken;
  *this = m_secretClient->GetDeletedSecrets(options, context);
  CurrentPageToken = options.NextPageToken.Value();
}

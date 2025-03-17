// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
/**
 * @file
 * @brief Defines SecretPropertiesPagedResponse.
 *
 */

#include "azure/keyvault/secrets/keyvault_secret_paged_response.hpp"

#include "./generated/key_vault_client_paged_responses.hpp"
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

SecretPropertiesPagedResponse::SecretPropertiesPagedResponse(
    _detail::GetSecretsPagedResponse& secretPagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<SecretClient> secretClient,
    std::string const& secretName)
    : m_secretName(secretName), m_secretClient(std::move(secretClient)),
      m_generatedResponse(
          std::make_shared<_detail::GetSecretsPagedResponse>(std::move(secretPagedResponse)))
{
  for (auto& item : m_generatedResponse->Value.Value())
  {
    Items.push_back(KeyVaultSecret(item).Properties);
  }
  RawResponse = std::move(rawResponse);
}

SecretPropertiesPagedResponse::SecretPropertiesPagedResponse(
    _detail::GetSecretVersionsPagedResponse& secretPagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<SecretClient> secretClient,
    std::string const& secretName)
    : m_secretName(secretName), m_secretClient(std::move(secretClient)),
      m_generatedVersionResponse(
          std::make_shared<_detail::GetSecretVersionsPagedResponse>(std::move(secretPagedResponse)))
{
  for (auto& item : m_generatedVersionResponse->Value.Value())
  {
    Items.push_back(KeyVaultSecret(item).Properties);
  }
  RawResponse = std::move(rawResponse);
}

DeletedSecretPagedResponse::DeletedSecretPagedResponse(
    _detail::GetDeletedSecretsPagedResponse& secretPagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<SecretClient> secretClient)
    : m_secretClient(std::move(secretClient)),
      m_generatedResponse(
          std::make_shared<_detail::GetDeletedSecretsPagedResponse>(std::move(secretPagedResponse)))
{
  for (auto& item : m_generatedResponse->Value.Value())
  {
    Items.push_back(DeletedSecret(item));
  }
  RawResponse = std::move(rawResponse);
}

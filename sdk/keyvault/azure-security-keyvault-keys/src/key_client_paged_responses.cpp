// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/key_client.hpp"
#include "azure/keyvault/keys/key_client_models.hpp"
#include "generated/key_vault_client_paged_responses.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;

void DeletedKeyPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  // Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  GetDeletedKeysOptions options;
  options.NextPageToken = NextPageToken;
  *this = m_keyClient->GetDeletedKeys(options, context);
  CurrentPageToken = options.NextPageToken.Value();
}

void KeyPropertiesPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  // Notes
  // - Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  // - KeyPropertiesPagedResponse is used to list keys from a Key Vault and also to list the key
  // versions from a specific key. When KeyPropertiesPagedResponse is listing keys, the `m_keyName`
  // fields will be empty, but for listing the key versions, the KeyPropertiesPagedResponse needs to
  // keep the name of the key in `m_keyName` because it is required to get more pages.
  //
  if (m_keyName.empty())
  {
    GetPropertiesOfKeysOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_keyClient->GetPropertiesOfKeys(options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
  else
  {
    GetPropertiesOfKeyVersionsOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_keyClient->GetPropertiesOfKeyVersions(m_keyName, options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
}

KeyPropertiesPagedResponse::KeyPropertiesPagedResponse(
    _detail::GetKeysPagedResponse const& pagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<KeyClient> keyClient,
    std::string const& keyName)
    : m_keyName(keyName), m_keyClient(std::move(keyClient))
{
  CurrentPageToken = pagedResponse.CurrentPageToken;
  NextPageToken = pagedResponse.NextPageToken;
  RawResponse = std::move(rawResponse);
  if (pagedResponse.Value.HasValue())
  {
    for (auto item : pagedResponse.Value.Value())
    {
      Items.emplace_back(KeyProperties(item));
    }
  }
}

KeyPropertiesPagedResponse::KeyPropertiesPagedResponse(
    _detail::GetKeyVersionsPagedResponse const& pagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<KeyClient> keyClient,
    std::string const& keyName)
    : m_keyName(keyName), m_keyClient(std::move(keyClient))
{
  CurrentPageToken = pagedResponse.CurrentPageToken;
  NextPageToken = pagedResponse.NextPageToken;
  RawResponse = std::move(rawResponse);
  if (pagedResponse.Value.HasValue())
  {
    for (auto item : pagedResponse.Value.Value())
    {
      Items.emplace_back(KeyProperties(item));
    }
  }
}

DeletedKeyPagedResponse::DeletedKeyPagedResponse(
    _detail::GetDeletedKeysPagedResponse&& pagedResponse,
    std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
    std::shared_ptr<KeyClient> keyClient)
    : m_keyClient(std::move(keyClient))
{
  CurrentPageToken = pagedResponse.CurrentPageToken;
  NextPageToken = pagedResponse.NextPageToken;
  RawResponse = std::move(rawResponse);
  if (pagedResponse.Value.HasValue())
  {
    for (auto item : pagedResponse.Value.Value())
    {
      Items.emplace_back(DeletedKey(item));
    }
  }
}

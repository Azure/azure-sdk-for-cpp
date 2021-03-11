// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json.hpp>

using namespace Azure::Security::KeyVault::Keys;
using Azure::Security::KeyVault::Common::_internal::UnixTimeConverter;

DeletedKey _detail::DeletedKeyDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto body = rawResponse.GetBody();
  auto jsonParser = Azure::Core::Json::_internal::json::parse(body);

  // "Key"
  DeletedKey deletedKey(name);
  _detail::KeyVaultKeyDeserialize(deletedKey, rawResponse);

  // recoveryId
  // deletedDate
  // scheduledPurgeDate
  deletedKey.RecoveryId = jsonParser[_detail::RecoveryIdPropertyName].get<std::string>();
  deletedKey.DeletedDate = UnixTimeConverter::UnixTimeToDatetime(
      jsonParser[_detail::DeletedOnPropertyName].get<uint64_t>());
  deletedKey.ScheduledPurgeDate = UnixTimeConverter::UnixTimeToDatetime(
      jsonParser[_detail::ScheduledPurgeDatePropertyName].get<uint64_t>());

  return deletedKey;
}

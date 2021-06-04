// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Security::KeyVault::_internal::UnixTimeConverter;

DeletedKey _detail::DeletedKeySerializer::DeletedKeyDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto body = rawResponse.GetBody();
  auto jsonParser = Azure::Core::Json::_internal::json::parse(body);

  // "Key"
  DeletedKey deletedKey(name);
  _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(deletedKey, rawResponse);

  // recoveryId
  // deletedDate
  // scheduledPurgeDate
  if (!jsonParser[_detail::RecoveryIdPropertyName].is_null())
  {
    deletedKey.RecoveryId = jsonParser[_detail::RecoveryIdPropertyName].get<std::string>();
  }
  if (!jsonParser[_detail::RecoveryLevelPropertyName].is_null())
  {
    deletedKey.Properties.RecoveryLevel
        = jsonParser[_detail::RecoveryLevelPropertyName].get<std::string>();
  }
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      deletedKey.DeletedDate,
      jsonParser,
      _detail::DeletedOnPropertyName,
      UnixTimeConverter::UnixTimeToDatetime);
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      deletedKey.ScheduledPurgeDate,
      jsonParser,
      _detail::ScheduledPurgeDatePropertyName,
      UnixTimeConverter::UnixTimeToDatetime);

  return deletedKey;
}

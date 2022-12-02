// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;

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
      PosixTimeConverter::PosixTimeToDateTime);
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      deletedKey.ScheduledPurgeDate,
      jsonParser,
      _detail::ScheduledPurgeDatePropertyName,
      PosixTimeConverter::PosixTimeToDateTime);

  return deletedKey;
}
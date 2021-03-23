// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/import_key_options.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Common::_internal;

std::string ImportKeyOptions::Serialize() const
{

  Azure::Core::Json::_internal::json payload;
  // key
  payload[_detail::KeyPropertyName] = Key;

  // hsm
  SetFromNullable(HardwareProtected, payload, _detail::HsmPropertyName);

  // attributes
  SetFromNullable<Azure::DateTime, uint64_t>(
      Properties.CreatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::CreatedPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable(
      Properties.Enabled, payload[_detail::AttributesPropertyName], _detail::EnabledPropertyName);
  SetFromNullable<Azure::DateTime, uint64_t>(
      Properties.ExpiresOn,
      payload[_detail::AttributesPropertyName],
      _detail::ExpPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable<Azure::DateTime, uint64_t>(
      Properties.NotBefore,
      payload[_detail::AttributesPropertyName],
      _detail::NbfPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable(
      Properties.RecoverableDays,
      payload[_detail::AttributesPropertyName],
      _detail::RecoverableDaysPropertyName);
  payload[_detail::RecoveryLevelPropertyName] = Properties.RecoveryLevel;
  SetFromNullable<Azure::DateTime, uint64_t>(
      Properties.UpdatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::UpdatedPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);

  // tags
  for (auto& tag : Properties.Tags)
  {
    payload[_detail::TagsPropertyName][tag.first] = tag.second;
  }

  // release_policy
  return payload.dump();
}

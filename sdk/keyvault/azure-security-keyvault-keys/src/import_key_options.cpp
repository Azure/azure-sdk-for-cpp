// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"
#include "azure/keyvault/keys/import_key_options.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Common::_internal;

std::string
Azure::Security::KeyVault::Keys::_detail::ImportKeyOptionsSerializer::ImportKeyOptionsSerialize(
    ImportKeyOptions const& importKeyOptions)
{

  Azure::Core::Json::_internal::json payload;
  // key
  payload[_detail::KeyPropertyName] = importKeyOptions.Key;

  // hsm
  SetFromNullable(importKeyOptions.HardwareProtected, payload, _detail::HsmPropertyName);

  // attributes
  SetFromNullable<Azure::DateTime, uint64_t>(
      importKeyOptions.Properties.CreatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::CreatedPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable(
      importKeyOptions.Properties.Enabled,
      payload[_detail::AttributesPropertyName],
      _detail::EnabledPropertyName);
  SetFromNullable<Azure::DateTime, uint64_t>(
      importKeyOptions.Properties.ExpiresOn,
      payload[_detail::AttributesPropertyName],
      _detail::ExpPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable<Azure::DateTime, uint64_t>(
      importKeyOptions.Properties.NotBefore,
      payload[_detail::AttributesPropertyName],
      _detail::NbfPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);
  SetFromNullable(
      importKeyOptions.Properties.RecoverableDays,
      payload[_detail::AttributesPropertyName],
      _detail::RecoverableDaysPropertyName);

  payload[_detail::RecoveryLevelPropertyName] = importKeyOptions.Properties.RecoveryLevel;

  SetFromNullable<Azure::DateTime, uint64_t>(
      importKeyOptions.Properties.UpdatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::UpdatedPropertyName,
      UnixTimeConverter::DatetimeToUnixTime);

  // tags
  for (auto& tag : importKeyOptions.Properties.Tags)
  {
    payload[_detail::TagsPropertyName][tag.first] = tag.second;
  }

  // release_policy
  return payload.dump();
}

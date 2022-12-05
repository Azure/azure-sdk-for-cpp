// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_options.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;

std::string
Azure::Security::KeyVault::Keys::_detail::ImportKeyOptionsSerializer::ImportKeyOptionsSerialize(
    ImportKeyOptions const& importKeyOptions)
{
  using Azure::Core::_internal::PosixTimeConverter;

  Azure::Core::Json::_internal::json payload;
  // key
  JsonWebKeySerializer::JsonWebKeySerialize(
      importKeyOptions.Key, payload[_detail::KeyPropertyName]);

  // hsm
  JsonOptional::SetFromNullable(
      importKeyOptions.HardwareProtected, payload, _detail::HsmPropertyName);

  // attributes
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      importKeyOptions.Properties.CreatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::CreatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(
      importKeyOptions.Properties.Enabled,
      payload[_detail::AttributesPropertyName],
      _detail::EnabledPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      importKeyOptions.Properties.ExpiresOn,
      payload[_detail::AttributesPropertyName],
      _detail::ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      importKeyOptions.Properties.NotBefore,
      payload[_detail::AttributesPropertyName],
      _detail::NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(
      importKeyOptions.Properties.RecoverableDays,
      payload[_detail::AttributesPropertyName],
      _detail::RecoverableDaysPropertyName);

  payload[_detail::RecoveryLevelPropertyName] = importKeyOptions.Properties.RecoveryLevel;

  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      importKeyOptions.Properties.UpdatedOn,
      payload[_detail::AttributesPropertyName],
      _detail::UpdatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  // tags
  for (auto& tag : importKeyOptions.Properties.Tags)
  {
    payload[_detail::TagsPropertyName][tag.first] = tag.second;
  }

  // release_policy
  JsonOptional::SetFromNullable<KeyReleasePolicy, Azure::Core::Json::_internal::json>(
      importKeyOptions.Properties.ReleasePolicy,
      payload,
      _detail::ReleasePolicyPropertyName,
      KeyReleasePolicySerializer::KeyReleasePolicySerialize);

  return payload.dump();
}
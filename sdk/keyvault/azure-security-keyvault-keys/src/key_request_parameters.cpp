// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "private/key_constants.hpp"
#include "private/key_request_parameters.hpp"
#include "private/key_serializers.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys::_detail;
using namespace Azure::Core::Json::_internal;

std::string KeyRequestParameters::Serialize() const
{
  using Azure::Core::_internal::PosixTimeConverter;

  Azure::Core::Json::_internal::json payload;
  // kty
  JsonOptional::SetFromNullable<KeyVaultKeyType, std::string>(
      m_keyType, payload, _detail::KeyTypePropertyName, [](KeyVaultKeyType type) {
        return type.ToString();
      });

  // attributes
  JsonOptional::SetFromNullable(
      m_options.Enabled, payload[_detail::AttributesPropertyName], _detail::EnabledPropertyName);

  // exportable attribute
  JsonOptional::SetFromNullable(
      m_options.Exportable,
      payload[_detail::AttributesPropertyName],
      _detail::ExportablePropertyName);

  /* Optional */
  // key_size
  // public_exponent
  // key_ops
  for (KeyOperation op : m_options.KeyOperations)
  {
    payload[_detail::KeyOpsPropertyName].push_back(op.ToString());
  }

  // attributes
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      m_options.ExpiresOn,
      payload[_detail::AttributesPropertyName],
      _detail::ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      m_options.NotBefore,
      payload[_detail::AttributesPropertyName],
      _detail::NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  // tags
  for (auto tag : m_options.Tags)
  {
    payload[_detail::TagsPropertyName][tag.first] = tag.second;
  }

  // crv
  if (Curve.HasValue())
  {
    payload[_detail::CurveNamePropertyName] = Curve.Value().ToString();
  }

  // release_policy
  JsonOptional::SetFromNullable<KeyReleasePolicy, Azure::Core::Json::_internal::json>(
      m_options.ReleasePolicy,
      payload,
      _detail::ReleasePolicyPropertyName,
      KeyReleasePolicySerializer::KeyReleasePolicySerialize);

  return payload.dump();
}
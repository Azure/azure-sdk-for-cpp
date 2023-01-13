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

  json attributes;
  // attributes
  JsonOptional::SetFromNullable(m_options.Enabled, attributes, _detail::EnabledPropertyName);

  // exportable attribute
  JsonOptional::SetFromNullable(m_options.Exportable, attributes, _detail::ExportablePropertyName);

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
      attributes,
      _detail::ExpPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      m_options.NotBefore,
      attributes,
      _detail::NbfPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);

  // in order to avoid creating the "attributes":null json field.
  // The service deserializer on HSM really does not like that
  if (!attributes.empty())
  {
    payload[_detail::AttributesPropertyName] = attributes;
  }

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

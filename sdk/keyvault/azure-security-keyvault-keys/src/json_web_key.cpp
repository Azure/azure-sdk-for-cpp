// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/keyvault/common/internal/base64url.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/json_web_key.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Common::_internal;

void Azure::Security::KeyVault::Keys::to_json(
    Azure::Core::Json::_internal::json& j,
    JsonWebKey const& p)
{
  j[_detail::KeyTypePropertyName] = KeyType::KeyTypeToString(p.KeyType);
  j[_detail::NPropertyName] = Base64Url::Base64UrlEncode(p.N);
  j[_detail::EPropertyName] = Base64Url::Base64UrlEncode(p.E);
  j[_detail::DPropertyName] = Base64Url::Base64UrlEncode(p.D);
  j[_detail::DPPropertyName] = Base64Url::Base64UrlEncode(p.DP);
  j[_detail::DQPropertyName] = Base64Url::Base64UrlEncode(p.DQ);
  j[_detail::QIPropertyName] = Base64Url::Base64UrlEncode(p.QI);
  j[_detail::PPropertyName] = Base64Url::Base64UrlEncode(p.P);
  j[_detail::QPropertyName] = Base64Url::Base64UrlEncode(p.Q);
}

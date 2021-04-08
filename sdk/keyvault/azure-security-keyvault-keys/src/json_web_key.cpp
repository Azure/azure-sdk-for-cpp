// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json_optional.hpp>
#include <azure/keyvault/common/internal/base64url.hpp>

#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"
#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::_internal;

namespace {
void ParseStringOperationsToKeyOperations(
    std::vector<KeyOperation>& keyOperations,
    std::vector<std::string> const& stringOperations)
{
  for (std::string const& operation : stringOperations)
  {
    keyOperations.emplace_back(KeyOperation(operation));
  }
}

static inline void AssignBytesIfExists(
    Azure::Core::Json::_internal::json const& jsonKey,
    std::string const& keyName,
    std::vector<uint8_t>& destBytes)
{
  JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
      destBytes, jsonKey, keyName, [](std::string const& value) {
        return Base64Url::Base64UrlDecode(value);
      });
}
} // namespace

void Azure::Security::KeyVault::Keys::_detail::JsonWebKeySerializer::JsonWebKeySerialize(
    JsonWebKey const& jwk,
    Azure::Core::Json::_internal::json& destJson)
{
  destJson[_detail::KeyTypePropertyName] = KeyType::KeyTypeToString(jwk.KeyType);
  destJson[_detail::NPropertyName] = Base64Url::Base64UrlEncode(jwk.N);
  destJson[_detail::EPropertyName] = Base64Url::Base64UrlEncode(jwk.E);
  destJson[_detail::DPropertyName] = Base64Url::Base64UrlEncode(jwk.D);
  destJson[_detail::DPPropertyName] = Base64Url::Base64UrlEncode(jwk.DP);
  destJson[_detail::DQPropertyName] = Base64Url::Base64UrlEncode(jwk.DQ);
  destJson[_detail::QIPropertyName] = Base64Url::Base64UrlEncode(jwk.QI);
  destJson[_detail::PPropertyName] = Base64Url::Base64UrlEncode(jwk.P);
  destJson[_detail::QPropertyName] = Base64Url::Base64UrlEncode(jwk.Q);
}

void Azure::Security::KeyVault::Keys::_detail::JsonWebKeySerializer::JsonWebDeserialize(
    JsonWebKey& srcKey,
    Azure::Core::Json::_internal::json const& jsonParser)
{
  // "Key"
  if (jsonParser.contains(_detail::KeyPropertyName))
  {
    auto const& jsonKey = jsonParser[_detail ::KeyPropertyName];
    {
      // key_ops
      auto keyOperationVector
          = jsonKey[_detail::KeyOpsPropertyName].get<std::vector<std::string>>();
      std::vector<KeyOperation> keyOperations;
      ParseStringOperationsToKeyOperations(keyOperations, keyOperationVector);
      srcKey.SetKeyOperations(keyOperations);
    }
    srcKey.Id = jsonKey[_detail::KeyIdPropertyName].get<std::string>();
    srcKey.KeyType
        = KeyType::KeyTypeFromString(jsonKey[_detail::KeyTypePropertyName].get<std::string>());

    JsonOptional::SetIfExists<std::string, KeyCurveName>(
        srcKey.CurveName, jsonKey, _detail::CurveNamePropertyName, [](std::string const& keyName) {
          return KeyCurveName(keyName);
        });

    AssignBytesIfExists(jsonKey, _detail::NPropertyName, srcKey.N);
    AssignBytesIfExists(jsonKey, _detail::EPropertyName, srcKey.E);
    AssignBytesIfExists(jsonKey, _detail::DPPropertyName, srcKey.DP);
    AssignBytesIfExists(jsonKey, _detail::DQPropertyName, srcKey.DQ);
    AssignBytesIfExists(jsonKey, _detail::QIPropertyName, srcKey.QI);
    AssignBytesIfExists(jsonKey, _detail::PPropertyName, srcKey.P);
    AssignBytesIfExists(jsonKey, _detail::QPropertyName, srcKey.Q);
    AssignBytesIfExists(jsonKey, _detail::DPropertyName, srcKey.D);
    // AssignBytesIfExists(jsonKey, _detail::KPropertyName, srcKey.K);
    // AssignBytesIfExists(jsonKey, _detail::TPropertyName, srcKey.T);
    // AssignBytesIfExists(jsonKey, _detail::XPropertyName, srcKey.X);
    // AssignBytesIfExists(jsonKey, _detail::YPropertyName, srcKey.Y);
  }
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;

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

static inline void WriteJsonIfVectorHasData(
    std::vector<uint8_t> const& srcVector,
    Azure::Core::Json::_internal::json& jsonKey,
    std::string const& keyName)
{
  JsonOptional::SetFromIfPredicate<std::vector<uint8_t> const&>(
      srcVector,
      [](std::vector<uint8_t> const& value) { return value.size() > 0; },
      jsonKey,
      keyName,
      Base64Url::Base64UrlEncode);
}
} // namespace

void Azure::Security::KeyVault::Keys::_detail::JsonWebKeySerializer::JsonWebKeySerialize(
    JsonWebKey const& jwk,
    Azure::Core::Json::_internal::json& destJson)
{
  // kty
  destJson[_detail::KeyTypePropertyName] = jwk.KeyType.ToString();

  // ops
  for (KeyOperation op : jwk.KeyOperations())
  {
    destJson[_detail::KeyOpsPropertyName].push_back(op.ToString());
  }

  // curve name
  JsonOptional::SetFromNullable<KeyCurveName, std::string>(
      jwk.CurveName, destJson, _detail::CurveNamePropertyName, [](KeyCurveName const& value) {
        return value.ToString();
      });
  if (jwk.Id.length() > 0)
  {
    destJson[_detail::KeyIdPropertyName] = jwk.Id;
  }

  // fields
  WriteJsonIfVectorHasData(jwk.N, destJson, _detail::NPropertyName);
  WriteJsonIfVectorHasData(jwk.E, destJson, _detail::EPropertyName);
  WriteJsonIfVectorHasData(jwk.D, destJson, _detail::DPropertyName);
  WriteJsonIfVectorHasData(jwk.DP, destJson, _detail::DPPropertyName);
  WriteJsonIfVectorHasData(jwk.DQ, destJson, _detail::DQPropertyName);
  WriteJsonIfVectorHasData(jwk.QI, destJson, _detail::QIPropertyName);
  WriteJsonIfVectorHasData(jwk.P, destJson, _detail::PPropertyName);
  WriteJsonIfVectorHasData(jwk.Q, destJson, _detail::QPropertyName);
  WriteJsonIfVectorHasData(jwk.X, destJson, _detail::XPropertyName);
  WriteJsonIfVectorHasData(jwk.Y, destJson, _detail::YPropertyName);
  WriteJsonIfVectorHasData(jwk.K, destJson, _detail::KPropertyName);
  WriteJsonIfVectorHasData(jwk.T, destJson, _detail::TPropertyName);
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
    srcKey.KeyType = KeyVaultKeyType(jsonKey[_detail::KeyTypePropertyName].get<std::string>());

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
    AssignBytesIfExists(jsonKey, _detail::KPropertyName, srcKey.K);
    AssignBytesIfExists(jsonKey, _detail::TPropertyName, srcKey.T);
    AssignBytesIfExists(jsonKey, _detail::XPropertyName, srcKey.X);
    AssignBytesIfExists(jsonKey, _detail::YPropertyName, srcKey.Y);
  }
}
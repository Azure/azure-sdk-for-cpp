// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../src/private/secret_serializers.hpp"
#include "../src/private/secret_constants.hpp"

#include "secret_get_client_deserialize_test.hpp"
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/keyvault/secrets/secret_client.hpp>
#include <azure/keyvault/secrets/keyvault_secret_set_parameters.hpp>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;

TEST(KeyvaultSecretSetParametersSerializer, SetValue)
{
  KeyVaultSecretSetParameters params;

  params.Value = "dasda";

  std::string result
      = KeyvaultSecretSetParametersSerializer::KeyvaultSecretSetParametersSerialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value);
  EXPECT_EQ(jsonParser[TagsPropertyName], nullptr);
  EXPECT_EQ(jsonParser[AttributesPropertyName], nullptr);
  EXPECT_EQ(jsonParser[ContentTypePropertyName], nullptr);
}

TEST(KeyvaultSecretSetParametersSerializer, SetValueCT)
{
  KeyVaultSecretSetParameters params;

  params.Value = "dasda";
  params.ContentType = "ct";

  std::string result
      = KeyvaultSecretSetParametersSerializer::KeyvaultSecretSetParametersSerialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value);
  EXPECT_EQ(jsonParser[TagsPropertyName], nullptr);
  EXPECT_EQ(jsonParser[AttributesPropertyName], nullptr);
  EXPECT_EQ(jsonParser[ContentTypePropertyName], params.ContentType.Value());
}

TEST(KeyvaultSecretSetParametersSerializer, SetValueCTAttrTag)
{
  KeyVaultSecretSetParameters params;

  params.Value = "dasda";
  params.ContentType = "ct";
  params.Attributes = KeyvaultSecretAttributes();
  params.Attributes.Value().Enabled = true;
  params.Attributes.Value().Name = "goqu";
  params.Tags = std::unordered_map<std::string, std::string>{{"a", "b"}};

  std::string result
      = KeyvaultSecretSetParametersSerializer::KeyvaultSecretSetParametersSerialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value);
  EXPECT_EQ(jsonParser[TagsPropertyName]["a"], "b");
  EXPECT_EQ(jsonParser[AttributesPropertyName][EnabledPropertyName], true);
  EXPECT_EQ(jsonParser[AttributesPropertyName]["name"], nullptr);
  EXPECT_EQ(jsonParser[ContentTypePropertyName], params.ContentType.Value());
}
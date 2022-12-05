// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include "../src/private/secret_serializers.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include "secret_get_client_deserialize_test.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;

TEST(KeyvaultSecretSetParametersSerializer, SetValue)
{
  KeyVaultSecret params("name", "value");

  std::string result = SecretSerializer::Serialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value.Value());
  EXPECT_EQ(jsonParser[IdPropertyName], nullptr);
  EXPECT_EQ(jsonParser[ContentTypePropertyName], nullptr);
}

TEST(KeyvaultSecretSetParametersSerializer, SetValueCT)
{
  KeyVaultSecret params("name", "value");

  params.Properties.ContentType = "ct";

  std::string result = SecretSerializer::Serialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value.Value());
  EXPECT_EQ(jsonParser[ContentTypePropertyName], params.Properties.ContentType.Value());
}

TEST(KeyvaultSecretSetParametersSerializer, SetValueCTAttrTag)
{
  KeyVaultSecret params("name", "value");

  params.Properties.ContentType = "ct";
  params.Properties.Enabled = true;
  params.Properties.Tags = std::unordered_map<std::string, std::string>{{"a", "b"}};

  std::string result = SecretSerializer::Serialize(params);

  auto jsonParser = json::parse(result);

  EXPECT_EQ(jsonParser[ValuePropertyName], params.Value.Value());
  EXPECT_EQ(jsonParser[AttributesPropertyName][TagsPropertyName]["a"], "b");
  EXPECT_EQ(jsonParser[AttributesPropertyName][EnabledPropertyName], true);
  EXPECT_EQ(jsonParser[ContentTypePropertyName], params.Properties.ContentType.Value());
}
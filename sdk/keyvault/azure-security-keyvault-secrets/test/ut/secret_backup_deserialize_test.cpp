// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_backup_deserialize_test.hpp"
#include "../src/private/secret_serializers.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;
TEST(BackupSecretSerializer, EmptyValue)
{
  auto response = BackupHelpers::GetEmptyResponse();

  auto secret = _detail::BackupSecretSerializer::Deserialize(response);

  EXPECT_EQ(secret.Secret.size(), size_t(0));
}

TEST(BackupSecretSerializer, FullValue)
{
  auto response = BackupHelpers::GetFullResponse();

  auto secret = _detail::BackupSecretSerializer::Deserialize(response);

  EXPECT_EQ(secret.Secret.size(), size_t(10));
  std::string str(secret.Secret.begin(), secret.Secret.end());
  EXPECT_EQ(str, "my name is");
}

TEST(RestoreSecretSerializer, EmptyValue)
{
  std::string str = "";
  auto data = std::vector<uint8_t>(str.begin(), str.end());
  auto secret = _detail::RestoreSecretSerializer::Serialize(data);
  auto jsonParser = json::parse(secret);

  EXPECT_EQ(secret.size(), size_t(12));
  EXPECT_EQ(jsonParser["value"].get<std::string>().empty(), true);
}

TEST(RestoreSecretSerializer, SomeValue)
{
  std::string str = "my name is";

  auto data = std::vector<uint8_t>(str.begin(), str.end());
  auto secret = _detail::RestoreSecretSerializer::Serialize(data);
  auto jsonParser = json::parse(secret);

  EXPECT_EQ(secret.size(), size_t(26));
  // cspell: disable-next-line
  EXPECT_EQ(jsonParser["value"], "bXkgbmFtZSBpcw");
}
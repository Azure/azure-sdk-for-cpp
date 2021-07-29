// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_backup_deserialize_test.hpp"
#include "../src/private/secret_serializers.hpp"

#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;
TEST(KeyvaultBackupSecretSerializer, EmptyValue)
{
  auto response = BackupHelpers::GetEmptyResponse();

  auto secret = _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(response);

  EXPECT_EQ(secret.Secret.size(), size_t(0));
}

TEST(KeyvaultBackupSecretSerializer, FullValue)
{
  auto response = BackupHelpers::GetFullResponse();

  auto secret = _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(response);

  EXPECT_EQ(secret.Secret.size(), size_t(10));
  std::string str(secret.Secret.begin(), secret.Secret.end());
  EXPECT_EQ(str, "my name is");
}

TEST(KeyvaultRestoreSecretSerializer, EmptyValue)
{
  std::string str = "";
  BackupSecretResponse data;
  data.Secret = std::vector<uint8_t>(str.begin(), str.end());
  auto secret = _detail::KeyvaultRestoreSecretSerializer::KeyvaultRestoreSecretSerialize(data);
  auto jsonParser = json::parse(secret);

  EXPECT_EQ(secret.size(), size_t(12));
  EXPECT_EQ(jsonParser["value"].get<std::string>().empty(), true);
}

TEST(KeyvaultRestoreSecretSerializer, SomeValue)
{
  std::string str = "my name is";

  BackupSecretResponse data;
  data.Secret = std::vector<uint8_t>(str.begin(), str.end());
  auto secret = _detail::KeyvaultRestoreSecretSerializer::KeyvaultRestoreSecretSerialize(data);
  auto jsonParser = json::parse(secret);

  EXPECT_EQ(secret.size(), size_t(26));
  EXPECT_EQ(jsonParser["value"], "bXkgbmFtZSBpcw");
}

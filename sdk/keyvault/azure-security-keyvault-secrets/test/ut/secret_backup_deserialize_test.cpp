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

  EXPECT_EQ(secret.size(), 0);
}

TEST(KeyvaultBackupSecretSerializer, FullValue)
{
  auto response = BackupHelpers::GetFullResponse();

  auto secret = _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(response);

  EXPECT_EQ(secret.size(), 16);
  std::string str(secret.begin(), secret.end());
  EXPECT_EQ(str, "bXkgbmFtZSBpcw==");
}

TEST(KeyvaultBackupSecretSerializer, IncorrectValue)
{
  auto response = BackupHelpers::GetIncorrectResponse();

  auto secret = _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(response);

  EXPECT_EQ(secret.size(), 10);
}

TEST(KeyvaultRestoreSecretSerializer, EmptyValue)
{
  std::string str = "";

  auto secret = _detail::KeyvaultRestoreSecretSerializer::KeyvaultRestoreSecretSerialize(
      std::vector<uint8_t>(str.begin(), str.end()));
  auto jsonParser = json::parse(secret);
  EXPECT_EQ(secret.size(), 12);
  EXPECT_EQ(jsonParser["value"].get<std::string>().empty(), true);
}

TEST(KeyvaultRestoreSecretSerializer, SomeValue)
{
  std::string str = "my name is";

  auto secret = _detail::KeyvaultRestoreSecretSerializer::KeyvaultRestoreSecretSerialize(
      std::vector<uint8_t>(str.begin(), str.end()));
  auto jsonParser = json::parse(secret);
  EXPECT_EQ(secret.size(), 22);
  EXPECT_EQ(jsonParser["value"], "my name is");
}
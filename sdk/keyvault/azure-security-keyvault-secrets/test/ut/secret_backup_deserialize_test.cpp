// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "secret_backup_deserialize_test.hpp"
#include "../src/private/secret_serializers.hpp"

#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;

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

  EXPECT_EQ(secret.size(), 10);
  std::string str(secret.begin(), secret.end());
  EXPECT_EQ(str, "my name is");
}

TEST(KeyvaultBackupSecretSerializer, IncorrectValue)
{
  auto response = BackupHelpers::GetIncorrectResponse();

  auto secret = _detail::KeyvaultBackupSecretSerializer::KeyvaultBackupSecretDeserialize(response);

  EXPECT_EQ(secret.size(), 6);
}

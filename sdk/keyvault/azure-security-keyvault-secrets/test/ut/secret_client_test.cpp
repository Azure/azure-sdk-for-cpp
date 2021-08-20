// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"
#include "secret_client_base_test.hpp"
#include <gtest/gtest.h>
#include <cstddef>

using namespace std::chrono_literals;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
TEST(SecretClient, InitClient)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    EXPECT_NO_THROW(SecretClient SecretClient("vaultUrl", credential));
  }
  {
    SecretClientOptions options;
    options.Retry.MaxRetries = 10;
    EXPECT_NO_THROW(SecretClient secretClient("vaultUrl", credential, options));
  }
}

TEST(SecretClient, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    // 7.2
    EXPECT_NO_THROW(auto options = SecretClientOptions(ServiceVersion::V7_2);
                    SecretClient SecretClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.Version.ToString(), "7.2"););
  }
  {
    // arbitrary version
    EXPECT_NO_THROW(auto options = SecretClientOptions(ServiceVersion("1.0"));
                    SecretClient secretClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.Version.ToString(), "1.0"););
  }
}

TEST(SecretClient, GetUrl)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");

  auto url = "vaultUrl";
  SecretClient secretClient(url, credential);
  EXPECT_EQ(url, secretClient.GetUrl());
}

TEST_F(KeyVaultSecretClientTest, FirstCreateTest)
{
  auto secretName = "CreateSecretWithThisName";
  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    // Now get the key
    auto secretResponse = client.GetSecret(secretName);
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

TEST_F(KeyVaultSecretClientTest, SecondCreateTest)
{
  auto secretName = "CreateSecretWithThisName";
  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);
  std::string version1;
  std::string version2;
  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    version1 = secret.Properties.Version;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto secretResponse = client.SetSecret(secretName, "secretValue2");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    version2 = secret.Properties.Version;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto secretResponse = client.GetPropertiesOfSecretsVersions(secretName);
    EXPECT_EQ(secretResponse.Items.size(), size_t(2));
    EXPECT_EQ(secretResponse.Items[0].Version, version1);
    EXPECT_EQ(secretResponse.Items[1].Version, version2);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

TEST_F(KeyVaultSecretClientTest, UpdateTest)
{
  auto secretName = "CreateSecretWithThisName";
  SecretProperties properties;
  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    // Now get the key
    auto secretResponse = client.GetSecret(secretName);
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    properties = secret.Properties;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    properties.ContentType = "xyz";
    UpdateSecretPropertiesOptions options;
    auto secretResponse = client.UpdateSecretProperties(secretName, properties.Version, properties);
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
    EXPECT_EQ(secret.Properties.ContentType.Value(), properties.ContentType.Value());
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

TEST_F(KeyVaultSecretClientTest, BackupRestore)
{
  auto secretName = "CreateSecretWithThisName";
  std::vector<uint8_t> backupData;
  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto backup = client.BackupSecret(secretName);
    CheckValidResponse(backup);
    backupData = backup.Value.Secret;
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
    std::this_thread::sleep_for(1s);
  }
  {
    auto restore = client.RestoreSecretBackup(backupData);
    CheckValidResponse(restore);
    auto restored = restore.Value;
    EXPECT_EQ(restored.Name, secretName);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

TEST_F(KeyVaultSecretClientTest, Recover)
{
  auto secretName = "CreateSecretWithThisName";
  std::vector<uint8_t> backupData;
  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    EXPECT_EQ(operation.GetResumeToken(), secretName);
    EXPECT_EQ(operation.HasValue(), true);
    auto operationResult = operation.Value();
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
    EXPECT_EQ(operationResult.Name, secretName);
    EXPECT_EQ(operation.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
  }
  {
    auto operation = client.StartRecoverDeletedSecret(secretName);
    operation.PollUntilDone(1s);
    EXPECT_EQ(operation.GetResumeToken(), secretName);
    EXPECT_EQ(operation.HasValue(), true);
    auto operationResult = operation.Value();
    auto restoredSecret = client.GetSecret(secretName);
    auto secret = restoredSecret.Value;
    EXPECT_EQ(secret.Name, secretName);
    EXPECT_EQ(operationResult.Name, secretName);
    EXPECT_EQ(operation.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}
TEST_F(KeyVaultSecretClientTest, GetProperties)
{
  auto secretName = "CreateSecretWithThisName";
  auto secretName2 = "CreateSecretWithThisName2";

  auto const& client = GetClientForTest(
      ::testing::UnitTest::GetInstance()->current_test_info()->name(),
      Azure::Core::Test::TestMode::PLAYBACK);
  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto secretResponse = client.SetSecret(secretName2, "secretValue2");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName2);
  }
  {
    auto secretResponse = client.GetPropertiesOfSecrets();
    EXPECT_EQ(secretResponse.Items.size(), size_t(2));
    EXPECT_EQ(secretResponse.Items[0].Name, secretName);
    EXPECT_EQ(secretResponse.Items[1].Name, secretName2);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(1s);
  }
  {
    auto operation = client.StartDeleteSecret(secretName2);
    operation.PollUntilDone(1s);
  }
  {
    auto deletedResponse = client.GetDeletedSecrets();
    EXPECT_EQ(deletedResponse.Items.size(),size_t(2));
    EXPECT_EQ(deletedResponse.Items[0].Name, secretName);
    EXPECT_EQ(deletedResponse.Items[1].Name, secretName2);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName2);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

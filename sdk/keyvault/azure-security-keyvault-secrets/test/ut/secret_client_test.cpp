// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"
#include "secret_client_base_test.hpp"
#include <cstddef>
#include <gtest/gtest.h>

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
  auto secretName = "FirstCreateTest";
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

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
    operation.PollUntilDone(m_defaultWait);
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
  auto secretName = "SecondCreateTest";
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
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
    EXPECT_TRUE(
        secretResponse.Items[0].Version == version1 || secretResponse.Items[0].Version == version2);
    EXPECT_TRUE(
        secretResponse.Items[1].Version == version1 || secretResponse.Items[1].Version == version2);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
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
  auto secretName = "UpdateTest";
  SecretProperties properties;
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
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
    auto props = properties;
    auto secretResponse = client.UpdateSecretProperties(properties);
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
    EXPECT_EQ(secret.Properties.ContentType.Value(), properties.ContentType.Value());
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
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
  auto secretName = "BackupRestore";
  BackupSecretResult backupData;
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto backup = client.BackupSecret(secretName);
    CheckValidResponse(backup);
    backupData = backup.Value;
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
    auto deletedSecretResponse = client.GetDeletedSecret(secretName);
    CheckValidResponse(deletedSecretResponse);
    auto secret = deletedSecretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto purgedResponse = client.PurgeDeletedSecret(secretName);
    CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
    std::this_thread::sleep_for(m_defaultWait);
  }
  {
    auto restore = client.RestoreSecretBackup(backupData);
    CheckValidResponse(restore);
    auto restored = restore.Value;
    EXPECT_EQ(restored.Name, secretName);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
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
  auto secretName = "Recover";
  std::vector<uint8_t> backupData;
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());

  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Name, secretName);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
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
    operation.PollUntilDone(m_defaultWait);
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
    operation.PollUntilDone(m_defaultWait);
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
  auto secretName = "GetProperties";
  auto secretName2 = "GetProperties2";

  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
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
    EXPECT_TRUE(
        secretResponse.Items[0].Name == secretName || secretResponse.Items[0].Name == secretName2);
    EXPECT_TRUE(
        secretResponse.Items[1].Name == secretName || secretResponse.Items[1].Name == secretName2);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(m_defaultWait);
  }
  {
    auto operation = client.StartDeleteSecret(secretName2);
    operation.PollUntilDone(m_defaultWait);
  }
  {
    auto deletedResponse = client.GetDeletedSecrets();
    EXPECT_EQ(deletedResponse.Items.size(), size_t(2));
    EXPECT_TRUE(
        deletedResponse.Items[0].Name == secretName
        || deletedResponse.Items[0].Name == secretName2);
    EXPECT_TRUE(
        deletedResponse.Items[1].Name == secretName
        || deletedResponse.Items[1].Name == secretName2);
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

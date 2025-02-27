// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
    EXPECT_NO_THROW(SecretClient SecretClient("http://account.vault.azure.net", credential));
  }
  {
    SecretClientOptions options;
    options.Retry.MaxRetries = 10;
    EXPECT_NO_THROW(
        SecretClient secretClient("http://account.vault.azure.net", credential, options));
  }
}

TEST(SecretClient, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  // Default - 7.5
  EXPECT_NO_THROW(auto options = SecretClientOptions();
                  SecretClient SecretClient("http://account.vault.azure.net", credential, options);
                  EXPECT_EQ(options.ApiVersion, "7.6-preview.1"););

  // 7.4
  EXPECT_NO_THROW(auto options = SecretClientOptions(); options.ApiVersion = "7.4";
                  SecretClient SecretClient("http://account.vault.azure.net", credential, options);
                  EXPECT_EQ(options.ApiVersion, "7.4"););
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
  auto secretName = GetTestName();
  auto const& client = GetClientForTest(secretName);
  std::string secretValue{"secretValue"};
  {
    auto secretResponse = client.SetSecret(secretName, "secretValue");
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Value.Value(), secretValue);
  }
  {
    // Now get the key
    auto secretResponse = client.GetSecret(secretName);
    CheckValidResponse(secretResponse);
    auto secret = secretResponse.Value;
    EXPECT_EQ(secret.Value.Value(), secretValue);
  }
}

// TEST_F(KeyVaultSecretClientTest, SecondCreateTest)
//{
//   auto secretName = GetTestName();
//   auto const& client = GetClientForTest(secretName);
//   std::string secretValue{"secretValue"};
//   std::string secretValue2{"secretValue2"};
//   std::string version1;
//   std::string version2;
//   {
//     auto secretResponse = client.SetSecret(secretName, secretValue);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     version1 = secret.Properties.Version;
//     EXPECT_EQ(secret.Value.Value(), secretValue);
//   }
//   {
//     auto secretResponse = client.SetSecret(secretName, secretValue2);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     version2 = secret.Properties.Version;
//     EXPECT_EQ(secret.Value.Value(), secretValue2);
//   }
//   {
//     auto secretResponse = client.GetPropertiesOfSecretsVersions(secretName);
//     EXPECT_EQ(secretResponse.Items.size(), size_t(2));
//     EXPECT_TRUE(
//         secretResponse.Items[0].Version == version1 || secretResponse.Items[0].Version ==
//         version2);
//     EXPECT_TRUE(
//         secretResponse.Items[1].Version == version1 || secretResponse.Items[1].Version ==
//         version2);
//   }
//   {
//     auto operation = client.StartDeleteSecret(secretName);
//     operation.PollUntilDone(m_defaultWait);
//     auto deletedSecretResponse = client.GetDeletedSecret(secretName);
//     CheckValidResponse(deletedSecretResponse);
//     auto secret = deletedSecretResponse.Value;
//     EXPECT_EQ(secret.Name, secretName);
//   }
//   {
//     auto purgedResponse = client.PurgeDeletedSecret(secretName);
//     CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
//   }
// }
//
// TEST_F(KeyVaultSecretClientTest, DISABLED_UpdateTest)
//{
//   auto secretName = "UpdateTest";
//   SecretProperties properties;
//   auto const& client
//       = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name());
//   std::string secretValue{"secretValue"};
//   {
//     auto secretResponse = client.SetSecret(secretName, secretValue);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     EXPECT_EQ(secret.Value.Value(), secretValue);
//   }
//   {
//     // Now get the key
//     auto secretResponse = client.GetSecret(secretName);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     properties = secret.Properties;
//     EXPECT_EQ(secret.Value.Value(), secretValue);
//     EXPECT_EQ(properties.Name, secretName);
//   }
//   {
//     properties.ContentType = "xyz";
//     UpdateSecretPropertiesOptions options;
//     auto props = properties;
//     auto secretResponse = client.UpdateSecretProperties(properties);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     EXPECT_EQ(secret.Properties.Name, secretName);
//     EXPECT_EQ(secret.Properties.ContentType.Value(), properties.ContentType.Value());
//   }
//   {
//     auto operation = client.StartDeleteSecret(secretName);
//     operation.PollUntilDone(m_defaultWait);
//     auto deletedSecretResponse = client.GetDeletedSecret(secretName);
//     CheckValidResponse(deletedSecretResponse);
//     auto secret = deletedSecretResponse.Value;
//     EXPECT_EQ(secret.Name, secretName);
//   }
//   {
//     auto purgedResponse = client.PurgeDeletedSecret(secretName);
//     CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
//   }
// }
//
// TEST_F(KeyVaultSecretClientTest, BackupRestore)
//{
//   auto secretName = GetTestName();
//   BackupSecretResult backupData;
//   auto const& client = GetClientForTest(secretName);
//   std::string secretValue{"secretValue"};
//   {
//     auto secretResponse = client.SetSecret(secretName, secretValue);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     EXPECT_EQ(secret.Value.Value(), secretValue);
//   }
//   {
//     auto backup = client.BackupSecret(secretName);
//     CheckValidResponse(backup);
//     backupData = backup.Value;
//   }
//   {
//     auto operation = client.StartDeleteSecret(secretName);
//     operation.PollUntilDone(m_defaultWait);
//     auto deletedSecretResponse = client.GetDeletedSecret(secretName);
//     CheckValidResponse(deletedSecretResponse);
//     auto secret = deletedSecretResponse.Value;
//     EXPECT_EQ(secret.Name, secretName);
//   }
//   {
//     auto purgedResponse = client.PurgeDeletedSecret(secretName);
//     CheckValidResponse(purgedResponse, Azure::Core::Http::HttpStatusCode::NoContent);
//     TestSleep(m_defaultWait);
//   }
//   {
//     int retries = 15;
//     // before we restore we need to ensure the secret is purged.
//     // since we have no visibility into the purge status we will just wait and check for a few
//     // seconds
//     while (retries > 15)
//     {
//       try
//       {
//         client.GetDeletedSecret(secretName);
//       }
//       catch (Azure::Core::RequestFailedException const&)
//       {
//         std::cout << std::endl << "- Secret is gone";
//         break;
//       }
//       TestSleep(m_defaultWait);
//       retries--;
//     }
//
//     if (retries == 0)
//     {
//       std::cout << std::endl << "retries reached 0";
//       EXPECT_TRUE(false);
//     }
//   }
//   {
//     auto restore = client.RestoreSecretBackup(backupData);
//     CheckValidResponse(restore);
//     auto restored = restore.Value;
//     EXPECT_TRUE(restored.Id.length() > 0);
//   }
// }
//
// TEST_F(KeyVaultSecretClientTest, RecoverSecret)
//{
//   auto secretName = GetTestName();
//   std::vector<uint8_t> backupData;
//   auto const& client = GetClientForTest(secretName);
//   std::string secretValue{"secretValue"};
//   {
//     auto secretResponse = client.SetSecret(secretName, secretValue);
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     EXPECT_EQ(secret.Value.Value(), secretValue);
//   }
//   {
//     auto operation = client.StartDeleteSecret(secretName);
//     // double polling should not have an impact on the result
//     operation.PollUntilDone(m_defaultWait);
//     operation.PollUntilDone(m_defaultWait);
//     EXPECT_EQ(operation.GetResumeToken(), secretName);
//     EXPECT_EQ(operation.HasValue(), true);
//     auto operationResult = operation.Value();
//     auto deletedSecretResponse = client.GetDeletedSecret(secretName);
//     CheckValidResponse(deletedSecretResponse);
//     auto secret = deletedSecretResponse.Value;
//     EXPECT_EQ(secret.Name, secretName);
//     EXPECT_EQ(operationResult.Name, secretName);
//     EXPECT_EQ(operation.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
//   }
//   {
//     auto operation = client.StartRecoverDeletedSecret(secretName);
//     // double polling should not have an impact on the result
//     operation.PollUntilDone(m_defaultWait);
//     operation.PollUntilDone(m_defaultWait);
//     EXPECT_EQ(operation.GetResumeToken(), secretName);
//     EXPECT_EQ(operation.HasValue(), true);
//     auto operationResult = operation.Value();
//     auto restoredSecret = client.GetSecret(secretName);
//     auto secret = restoredSecret.Value;
//     EXPECT_EQ(secret.Name, secretName);
//     EXPECT_EQ(operationResult.Name, secretName);
//     EXPECT_EQ(operation.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
//   }
// }
// TEST_F(KeyVaultSecretClientTest, TestGetPropertiesOfSecret)
//{
//   std::string const testName(GetTestName());
//   auto const& client = GetClientForTest(testName);
//   int capacity = 10; // had to reduce size to workaround test-proxy issue with max payload size
//   // Create secrets
//   std::vector<std::string> secretNames;
//   for (int counter = 0; counter < capacity; counter++)
//   {
//     std::string const name(testName + std::to_string(counter));
//     secretNames.emplace_back(name);
//     auto secretResponse = client.SetSecret(name, "secretValue");
//     CheckValidResponse(secretResponse);
//     auto secret = secretResponse.Value;
//     EXPECT_EQ(secret.Name, name);
//     // Avoid server Throttled while creating keys
//     TestSleep();
//   }
//   // Get Secret properties
//   std::vector<SecretProperties> secretProps;
//
//   for (auto secretResponse = client.GetPropertiesOfSecrets(); secretResponse.HasPage();
//        secretResponse.MoveToNextPage())
//   {
//     for (auto& secret : secretResponse.Items)
//     {
//       secretProps.emplace_back(secret);
//     }
//   }
//
//   EXPECT_TRUE(secretProps.size() >= static_cast<size_t>(capacity));
// }

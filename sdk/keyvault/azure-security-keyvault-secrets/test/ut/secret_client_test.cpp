// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include "secret_client_base_test.hpp"
#include <stdlib.h>
#include <exception>
#include <gtest/gtest.h>
#include <memory>
#include <chrono>

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
    operation.PollUntilDone(2s);
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
    EXPECT_EQ(secretResponse.Items.size(), 2);
    EXPECT_EQ(secretResponse.Items[0].Version, version1);
    EXPECT_EQ(secretResponse.Items[1].Version, version2);
  }
  {
    auto operation = client.StartDeleteSecret(secretName);
    operation.PollUntilDone(2s);
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

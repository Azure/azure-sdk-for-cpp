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
  auto const& client
      = GetClientForTest(::testing::UnitTest::GetInstance()->current_test_info()->name(), Azure::Core::Test::TestMode::RECORD);

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
    // Now get the key
    auto secretResponse = client.StartDeleteSecret(secretName);
    
  }
}

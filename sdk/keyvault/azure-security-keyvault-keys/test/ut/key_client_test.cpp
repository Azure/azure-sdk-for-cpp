// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/keys.hpp>

#include <exception>
#include <memory>

using namespace Azure::Security::KeyVault::Keys;

TEST(KeyVaultKeyClientUnitTest, initClient)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    EXPECT_NO_THROW(KeyClient keyClient("http://account.vault.azure.net", credential));
  }
  {
    KeyClientOptions options;
    options.Retry.MaxRetries = 10;
    EXPECT_NO_THROW(KeyClient keyClient("http://account.vault.azure.net", credential, options));
  }
}

TEST(KeyVaultKeyClientUnitTest, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    // 7.2
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion::V7_2);
                    KeyClient keyClient("http://account.vault.azure.net", credential, options);
                    EXPECT_EQ(options.Version.ToString(), "7.2"););
  }
  {
    // arbitrary version
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion("1.0"));
                    KeyClient keyClient("http://account.vault.azure.net", credential, options);
                    EXPECT_EQ(options.Version.ToString(), "1.0"););
  }
}

TEST(KeyVaultKeyClientUnitTest, GetUrl)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");

  auto url = "vaultUrl";
  KeyClient keyClient(url, credential);
  EXPECT_EQ(url, keyClient.GetUrl());
}

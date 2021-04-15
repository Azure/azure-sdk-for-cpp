// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <exception>
#include <memory>

using namespace Azure::Security::KeyVault::Keys;

TEST(KeyClient, initClient)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    EXPECT_NO_THROW(KeyClient keyClient("vaultUrl", credential));
  }
  {
    KeyClientOptions options;
    options.Retry.MaxRetries = 10;
    EXPECT_NO_THROW(KeyClient keyClient("vaultUrl", credential, options));
  }
}

TEST(KeyClient, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  {
    // 7.0
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion::V7_0);
                    KeyClient keyClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.GetVersionString(), "7.0"););
  }
  {
    // 7.1
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion::V7_1);
                    KeyClient keyClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.GetVersionString(), "7.1"););
  }
  {
    // 7.2
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion::V7_2);
                    KeyClient keyClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.GetVersionString(), "7.2"););
  }
  {
    // arbitrary version
    EXPECT_NO_THROW(auto options = KeyClientOptions(ServiceVersion("1.0"));
                    KeyClient keyClient("vaultUrl", credential, options);
                    EXPECT_EQ(options.GetVersionString(), "1.0"););
  }
}

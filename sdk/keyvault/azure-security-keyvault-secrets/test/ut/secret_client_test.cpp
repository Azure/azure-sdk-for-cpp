// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"

#include <exception>
#include <gtest/gtest.h>
#include <memory>

using namespace Azure::Security::KeyVault::Secrets;

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
  {
    // 7.2
    EXPECT_NO_THROW(
        auto options = SecretClientOptions(ServiceVersion::V7_2);
        SecretClient SecretClient("http://account.vault.azure.net", credential, options);
        EXPECT_EQ(options.Version.ToString(), "7.2"););
  }
  {
    // arbitrary version
    EXPECT_NO_THROW(
        auto options = SecretClientOptions(ServiceVersion("1.0"));
        SecretClient secretClient("http://account.vault.azure.net", credential, options);
        EXPECT_EQ(options.Version.ToString(), "1.0"););
  }
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/key_vault.hpp>

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
    EXPECT_NO_THROW(KeyClient keyClient("vaultUrl", credential));
  }
}

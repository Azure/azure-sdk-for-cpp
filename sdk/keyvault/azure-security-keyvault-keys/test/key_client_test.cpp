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
  EXPECT_NO_THROW(KeyClient keyClient("vaultUrl", credential));
}

TEST(KeyClient, DISABLED_SendRequestDefault)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  KeyClient keyClient("vaultUrl", credential);
  auto r = keyClient.GetKey(Azure::Core::GetApplicationContext(), "KeyName");
  auto t = r.ExtractValue();
  auto rr = r.ExtractRawResponse();

  EXPECT_EQ(t.Name(), "KeyName");
}

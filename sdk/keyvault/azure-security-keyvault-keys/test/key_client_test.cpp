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

TEST(KeyClient, DISABLED_GetKey)
{
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      "tenantId", "clientId", "secrectId");
  KeyClient keyClient("https://vivazqukeyvault.vault.azure.net/", credential);
  auto r = keyClient.GetKey("KeyName");
  auto keyVaultKey = r.ExtractValue();

  EXPECT_EQ(keyVaultKey.Name(), "KeyName");
}

TEST(KeyClient, DISABLED_CreateKey)
{
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      "tenantId", "clientId", "secrectId");
  KeyClient keyClient("https://vivazqukeyvault.vault.azure.net/", credential);
  auto r = keyClient.CreateKey("myKey2", KeyTypeEnum::Ec);
  auto keyVaultKey = r.ExtractValue();

  EXPECT_EQ(keyVaultKey.Name(), "myKey2");
}

TEST(KeyClient, DISABLED_CreateKeyWithOptions)
{
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      "tenantId", "clientId", "secrectId");
  KeyClient keyClient("https://vivazqukeyvault.vault.azure.net/", credential);

  CreateKeyOptions options;
  options.KeyOperations.push_back(KeyOperation::Sign());
  options.KeyOperations.push_back(KeyOperation::Verify());

  auto r = keyClient.CreateKey("myKeyWithOptions2", KeyTypeEnum::Ec, options);
  auto keyVaultKey = r.ExtractValue();

  EXPECT_EQ(keyVaultKey.Name(), "myKeyWithOptions2");
  EXPECT_EQ(keyVaultKey.GetKeyType(), KeyTypeEnum::Ec);
  auto& keyOperations = keyVaultKey.KeyOperations();
  uint16_t const expected = 2;
  EXPECT_EQ(keyOperations.size(), expected);

  auto findOperation = [keyOperations](KeyOperation op) {
    for (KeyOperation operation : keyOperations)
    {
      if (operation.ToString() == op.ToString())
      {
        return true;
      }
    }
    return false;
  };
  EXPECT_PRED1(findOperation, KeyOperation::Sign());
  EXPECT_PRED1(findOperation, KeyOperation::Verify());
}

TEST(KeyClient, DISABLED_CreateKeyWithTags)
{
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      "tenantId", "clientId", "secrectId");
  KeyClient keyClient("https://vivazqukeyvault.vault.azure.net/", credential);

  CreateKeyOptions options;
  options.Tags.emplace("one", "value=1");
  options.Tags.emplace("two", "value=2");

  auto r = keyClient.CreateKey("myKeyWithOptionsTags", KeyTypeEnum::Rsa, options);
  auto keyVaultKey = r.ExtractValue();

  EXPECT_EQ(keyVaultKey.Name(), "myKeyWithOptionsTags");
  EXPECT_EQ(keyVaultKey.GetKeyType(), KeyTypeEnum::Rsa);

  auto findTag = [keyVaultKey](std::string key, std::string value) {
    // Will throw if key is not found
    auto v = keyVaultKey.Properties.Tags.at(key);
    return value == v;
  };
  EXPECT_PRED2(findTag, "one", "value=1");
  EXPECT_PRED2(findTag, "two", "value=2");
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultClientTest, GetKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Assuming and RS Key exists in the KeyVault Account.
  std::string keyName("testKey");

  auto r = keyClient.GetKey(keyName);
  auto key = r.ExtractValue();

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), Azure::Security::KeyVault::Keys::KeyTypeEnum::Rsa);
}

TEST_F(KeyVaultClientTest, CreateKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName("createKey");

  {
    auto key = keyClient.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyTypeEnum::Ec);
    auto keyVaultKey = key.ExtractValue();
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto key = keyClient.GetKey(keyName);
    auto keyVaultKey = key.ExtractValue();
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

// TEST_F(KeyVaultClientTest, DISABLED_CreateKeyWithOptions)
// {
//   auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
//       "tenantId", "clientId", "secrectId");
//   Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);

//   Azure::Security::KeyVault::Keys::CreateKeyOptions options;
//   options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign());
//   options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify());

//   auto r = keyClient.CreateKey("myKeyWithOptions2", KeyTypeEnum::Ec, options);
//   auto keyVaultKey = r.ExtractValue();

//   EXPECT_EQ(keyVaultKey.Name(), "myKeyWithOptions2");
//   EXPECT_EQ(keyVaultKey.GetKeyType(), KeyTypeEnum::Ec);
//   auto& keyOperations = keyVaultKey.KeyOperations();
//   EXPECT_EQ(keyOperations.size(), 2);

//   auto findOperation = [keyOperations](KeyOperation op) {
//     for (KeyOperation operation : keyOperations)
//     {
//       if (operation.ToString() == op.ToString())
//       {
//         return true;
//       }
//     }
//     return false;
//   };
//   EXPECT_PRED1(findOperation, KeyOperation::Sign());
//   EXPECT_PRED1(findOperation, KeyOperation::Verify());
// }

// TEST_F(KeyVaultClientTest, DISABLED_CreateKeyWithTags)
// {
//   auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
//       "tenantId", "clientId", "secrectId");
//   Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);

//   CreateKeyOptions options;
//   options.Tags.emplace("one", "value=1");
//   options.Tags.emplace("two", "value=2");

//   auto r = keyClient.CreateKey("myKeyWithOptionsTags", KeyTypeEnum::Rsa, options);
//   auto keyVaultKey = r.ExtractValue();

//   EXPECT_EQ(keyVaultKey.Name(), "myKeyWithOptionsTags");
//   EXPECT_EQ(keyVaultKey.GetKeyType(), KeyTypeEnum::Rsa);

//   auto findTag = [keyVaultKey](std::string key, std::string value) {
//     // Will throw if key is not found
//     auto v = keyVaultKey.Properties.Tags.at(key);
//     return value == v;
//   };

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/details/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;

TEST_F(KeyVaultClientTest, GetKey)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Assuming and RS Key exists in the KeyVault Account.
  std::string keyName("testKey");

  auto keyResponse = keyClient.GetKey(keyName);
  CheckValidResponse(keyResponse);
  auto key = keyResponse.ExtractValue();

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), JsonWebKeyType::Rsa);
}

#include <iostream>
TEST_F(KeyVaultClientTest, GetPropertiesOfKeysOnePage)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);

  auto keyResponse = keyClient.GetPropertiesOfKeysSinglePage();
  CheckValidResponse(keyResponse);
  for (auto const& keyProperties : keyResponse->Items)
  {
    auto keyVersions = keyClient.GetPropertiesOfKeyVersions(keyProperties.Name);
    CheckValidResponse(keyVersions);
    std::cout << std::endl
              << keyProperties.Name << " - " << keyProperties.CreatedOn.GetValue().ToString();
    std::cout << std::endl << "Versions:";
    for (auto const& keyVersion : keyVersions->Items)
    {
      std::cout << std::endl << "\t-" << keyVersion.Version;
    }
  }
}

TEST_F(KeyVaultClientTest, GetDeletedKeysOnePage)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);

  // Create 5 keys
  std::vector<std::string> keyNames;
  for (int counter = 0; counter < 5; counter++)
  {
    auto name = GetUniqueName();
    CreateEcKeyOptions options(name);
    keyNames.emplace_back(name);
    auto response = keyClient.CreateEcKey(options);
    CheckValidResponse(response);
  }
  // Delete keys
  std::vector<DeleteKeyOperation> operations;
  for (auto const& keyName : keyNames)
  {
    operations.emplace_back(keyClient.StartDeleteKey(keyName));
  }
  // wait for all of the delete operations to complete
  for (auto& operation : operations)
  {
    operation.PollUntilDone(std::chrono::milliseconds(1000));
  }

  // Get all deleted Keys
  std::vector<DeletedKey> deletedKeys;
  GetDeletedKeysOptions options;
  while (true)
  {
    auto keyResponse = keyClient.GetDeletedKeysSinglePage(options);
    for (auto& key : keyResponse->Items)
    {
      deletedKeys.emplace_back(key);
    }
    if (!keyResponse->ContinuationToken)
    {
      break;
    }
    options.ContinuationToken = keyResponse->ContinuationToken;
  }

  EXPECT_EQ(keyNames.size(), deletedKeys.size());
  for (auto const& deletedKey : deletedKeys)
  {
    std::cout << std::endl << deletedKey.Id();
  }

  // Purge
  for (auto const& keyName : keyNames)
  {
    keyClient.PurgeDeletedKey(keyName);
  }
}

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

TEST_F(KeyVaultClientTest, GetSingleKey)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Assuming and RS Key exists in the KeyVault Account.
  std::string keyName(GetUniqueName());

  auto createKeyResponse = keyClient.CreateEcKey(CreateEcKeyOptions(keyName));
  CheckValidResponse(createKeyResponse);
  auto keyResponse = keyClient.GetKey(keyName);
  CheckValidResponse(keyResponse);
  auto key = keyResponse.Value;

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), JsonWebKeyType::Ec);
}

TEST_F(KeyVaultClientTest, GetPropertiesOfKeysOnePage)
{

  KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Delete and purge anything before starting the test to ensure test will work
  RemoveAllKeysFromVault(keyClient);

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
  // Get Key properties
  std::vector<KeyProperties> keyPropertiesList;
  GetPropertiesOfKeysSinglePageOptions options;
  while (true)
  {
    auto keyResponse = keyClient.GetPropertiesOfKeysSinglePage(options);
    for (auto& key : keyResponse->Items)
    {
      keyPropertiesList.emplace_back(key);
    }
    if (!keyResponse->ContinuationToken)
    {
      break;
    }
    options.ContinuationToken = keyResponse->ContinuationToken;
  }

  EXPECT_EQ(keyNames.size(), keyPropertiesList.size());
  for (auto const& keyProperties : keyPropertiesList)
  {
    // Check names are in the keyNames list
    auto findKeyName = std::find(keyNames.begin(), keyNames.end(), keyProperties.Name);
    EXPECT_NE(findKeyName, keyNames.end());
  }

  // Clean vault
  RemoveAllKeysFromVault(keyClient, false);
}

TEST_F(KeyVaultClientTest, GetKeysVersionsOnePage)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);

  // Create 5 key versions
  std::string keyName(GetUniqueName());
  size_t expectedVersions = 5;
  CreateEcKeyOptions createKeyOptions(keyName);
  for (size_t counter = 0; counter < expectedVersions; counter++)
  {
    auto response = keyClient.CreateEcKey(createKeyOptions);
    CheckValidResponse(response);
  }
  // Get Key versions
  std::vector<KeyProperties> keyPropertiesList;
  GetPropertiesOfKeyVersionsSinglePageOptions getKeyOptions;
  while (true)
  {
    auto keyResponse = keyClient.GetPropertiesOfKeyVersionsSinglePage(keyName, getKeyOptions);
    for (auto& key : keyResponse->Items)
    {
      keyPropertiesList.emplace_back(key);
    }
    if (!keyResponse->ContinuationToken)
    {
      break;
    }
    getKeyOptions.ContinuationToken = keyResponse->ContinuationToken;
  }

  EXPECT_EQ(expectedVersions, keyPropertiesList.size());
  for (auto const& keyProperties : keyPropertiesList)
  {
    EXPECT_EQ(keyName, keyProperties.Name);
  }

  // Clean vault
  RemoveAllKeysFromVault(keyClient, false);
}

TEST_F(KeyVaultClientTest, GetDeletedKeysOnePage)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);

  // Delete and purge anything before starting the test to ensure test will work
  CleanUpKeyVault(keyClient);

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
  GetDeletedKeysSinglePageOptions options;
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
    // Check names are in the keyNames list
    auto findKeyName = std::find(keyNames.begin(), keyNames.end(), deletedKey.Name());
    EXPECT_NE(findKeyName, keyNames.end());
  }

  // Purge
  for (auto const& keyName : keyNames)
  {
    keyClient.PurgeDeletedKey(keyName);
  }
}

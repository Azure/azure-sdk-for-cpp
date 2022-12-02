// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;

TEST_F(KeyVaultKeyClient, GetSingleKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  auto createKeyResponse = client.CreateEcKey(CreateEcKeyOptions(keyName));
  CheckValidResponse(createKeyResponse);
  auto keyResponse = client.GetKey(keyName);
  CheckValidResponse(keyResponse);
  auto key = keyResponse.Value;

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), KeyVaultKeyType::Ec);
}

TEST_F(KeyVaultKeyClient, GetPropertiesOfKeysAllPages)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  // Create 5 keys
  std::vector<std::string> keyNames;
  for (int counter = 0; counter < 50; counter++)
  {
    std::string const name(keyName + std::to_string(counter));
    CreateEcKeyOptions options(name);
    keyNames.emplace_back(name);
    auto response = client.CreateEcKey(options);
    // Avoid server Throttled while creating keys
    TestSleep();
    CheckValidResponse(response);
  }
  // Get Key properties
  std::vector<KeyProperties> keyPropertiesList;
  GetPropertiesOfKeysOptions options;
  for (auto keyResponse = client.GetPropertiesOfKeys(options); keyResponse.HasPage();
       keyResponse.MoveToNextPage())
  {
    for (auto& key : keyResponse.Items)
    {
      keyPropertiesList.emplace_back(key);
    }
  }

  for (auto const& key : keyNames)
  {
    // Check names are in the returned list
    auto findKeyName = std::find_if(
        keyPropertiesList.begin(),
        keyPropertiesList.end(),
        [&key](KeyProperties const& returnedKey) { return returnedKey.Name == key; });
    EXPECT_NE(findKeyName, keyPropertiesList.end());
    EXPECT_EQ(key, findKeyName->Name);
  }
}

TEST_F(KeyVaultKeyClient, GetKeysVersions)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  // Create 5 key versions
  size_t expectedVersions = 50;
  CreateEcKeyOptions createKeyOptions(keyName);
  for (size_t counter = 0; counter < expectedVersions; counter++)
  {
    auto response = client.CreateEcKey(createKeyOptions);
    CheckValidResponse(response);
    EXPECT_NE(response.Value.Properties.Version, std::string(""));
    // Avoid server Throttled while creating keys
    TestSleep();
  }

  // Get Key versions
  std::vector<KeyProperties> keyPropertiesList;
  GetPropertiesOfKeyVersionsOptions getKeyOptions;
  for (auto keyResponse = client.GetPropertiesOfKeyVersions(keyName); keyResponse.HasPage();
       keyResponse.MoveToNextPage())
  {
    for (auto& key : keyResponse.Items)
    {
      keyPropertiesList.emplace_back(key);
    }
  }

  EXPECT_EQ(expectedVersions, keyPropertiesList.size());
  for (auto const& keyProperties : keyPropertiesList)
  {
    EXPECT_EQ(keyName, keyProperties.Name);
    // Check we can get key version from server
    GetKeyOptions options;
    options.Version = keyProperties.Version;
    auto versionedKey = client.GetKey(keyProperties.Name, options);
    CheckValidResponse(versionedKey);
    EXPECT_EQ(keyProperties.Version, versionedKey.Value.Properties.Version);
    // Avoid server Throttled while creating keys
    TestSleep();
  }
}

TEST_F(KeyVaultKeyClient, GetDeletedKeys)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  // Create 5 keys
  std::vector<std::string> keyNames;
  for (int counter = 0; counter < 50; counter++)
  {
    std::string const name(keyName + std::to_string(counter));
    CreateEcKeyOptions options(name);
    keyNames.emplace_back(name);
    auto response = client.CreateEcKey(options);
    CheckValidResponse(response);
    // Avoid server Throttled while creating keys
    TestSleep();
  }
  // Delete keys
  std::vector<DeleteKeyOperation> operations;
  for (auto const& key : keyNames)
  {
    operations.emplace_back(client.StartDeleteKey(key));
    // Avoid server Throttled while creating keys
    TestSleep();
  }
  // wait for all of the delete operations to complete
  for (auto& operation : operations)
  {
    operation.PollUntilDone(m_testPollingIntervalMs);
  }

  // Get all deleted Keys
  std::vector<std::string> deletedKeys;
  for (auto keyResponse = client.GetDeletedKeys(); keyResponse.HasPage();
       keyResponse.MoveToNextPage())
  {
    for (auto& key : keyResponse.Items)
    {
      deletedKeys.emplace_back(key.Name());
    }
  }

  // Check all keys are in the deleted key list
  for (auto const& key : keyNames)
  {
    // Check names are in the keyNames list
    auto findKeyName = std::find(deletedKeys.begin(), deletedKeys.end(), key);
    EXPECT_NE(findKeyName, deletedKeys.end());
  }
}
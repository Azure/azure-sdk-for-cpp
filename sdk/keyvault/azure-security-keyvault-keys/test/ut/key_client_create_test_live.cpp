// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultKeyClient, CreateKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultKeyClient, CreateKeyWithOptions)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign);
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify);
  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(
        keyVaultKey.GetKeyType().ToString(),
        Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec.ToString());
    auto& keyOperations = keyVaultKey.KeyOperations();
    uint16_t expectedSize = 2;
    EXPECT_EQ(keyOperations.size(), expectedSize);

    auto findOperation = [keyOperations](Azure::Security::KeyVault::Keys::KeyOperation op) {
      for (Azure::Security::KeyVault::Keys::KeyOperation operation : keyOperations)
      {
        if (operation.ToString() == op.ToString())
        {
          return true;
        }
      }
      return false;
    };
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Sign);
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Verify);
  }
}

TEST_F(KeyVaultKeyClient, CreateKeyWithTags)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.Tags.emplace("one", "value=1");
  options.Tags.emplace("two", "value=2");

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Rsa, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(keyVaultKey.GetKeyType(), Azure::Security::KeyVault::Keys::KeyVaultKeyType::Rsa);

    auto findTag = [keyVaultKey](std::string key, std::string value) {
      // Will throw if key is not found
      auto v = keyVaultKey.Properties.Tags.at(key);
      return value == v;
    };
  }
}

/********************************* Create key overloads  *********************************/
TEST_F(KeyVaultKeyClient, CreateEcKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName);
    auto keyResponse = client.CreateEcKey(ecKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultKeyClient, CreateEcKeyWithCurve)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName);
    ecKey.CurveName = Azure::Security::KeyVault::Keys::KeyCurveName::P384;
    auto keyResponse = client.CreateEcKey(ecKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(ecKey.CurveName->ToString(), keyVaultKey.Key.CurveName->ToString());
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Key.CurveName.HasValue());
    EXPECT_EQ(
        keyVaultKey.Key.CurveName.Value().ToString(),
        Azure::Security::KeyVault::Keys::KeyCurveName::P384.ToString());
  }
}

TEST_F(KeyVaultKeyClient, CreateRsaKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto rsaKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, false);
    auto keyResponse = client.CreateRsaKey(rsaKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

// No tests for octKey since the server does not support it.

TEST_F(KeyVaultKeyClient, CreateEcHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecHsmKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName, true);
    auto keyResponse = client.CreateEcKey(ecHsmKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultKeyClient, CreateRsaHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto rsaHsmKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, true);
    auto keyResponse = client.CreateRsaKey(rsaHsmKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

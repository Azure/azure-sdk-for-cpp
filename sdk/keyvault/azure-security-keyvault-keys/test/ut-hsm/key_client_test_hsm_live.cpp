// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "key_client_test_hsm_live.hpp"

#include "gtest/gtest.h"
#include <azure/keyvault/keys.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Test;

// No tests for octKey since the server does not support it.
// FOR THIS TEST TO WORK MAKE SURE YOU ACTUALLY HAVE A VALID HSM VALUE FOR AZURE_KEYVAULT_HSM_URL
TEST_F(KeyVaultKeyHSMClient, CreateEcHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecHsmKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName, true);
    ecHsmKey.Enabled = true;
    ecHsmKey.KeyOperations = {KeyOperation::Sign};
    auto keyResponse = client.CreateEcKey(ecHsmKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_FALSE(keyResponse.Value.Properties.ReleasePolicy.HasValue());
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
}
// FOR THIS TEST TO WORK MAKE SURE YOU ACTUALLY HAVE A VALID HSM VALUE FOR AZURE_KEYVAULT_HSM_URL
TEST_F(KeyVaultKeyHSMClient, CreateRsaHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);
  {
    auto rsaHsmKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, true);
    rsaHsmKey.Enabled = true;
    rsaHsmKey.KeyOperations = {KeyOperation::Sign};
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
    EXPECT_FALSE(keyResponse.Value.Properties.ReleasePolicy.HasValue());
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
}


TEST_F(KeyVaultKeyHSMClient, GetRandomBytes)
{
  if (m_keyVaultUrl.compare(m_keyVaultHsmUrl) != 0)
  {
    auto const keyName = GetTestName();
    CreateHsmClient();
    auto const& client = GetClientForTest(keyName);
    GetRandomBytesOptions options;
    options.Count = 4;
    auto result = client.GetRandomBytes(options);
    EXPECT_EQ(result.Value.RandomBytes.size(), size_t(options.Count));
  }
  EXPECT_TRUE(true);
}
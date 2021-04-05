// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/core/datetime.hpp>
#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/details/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure;
using namespace Azure::Security::KeyVault::Keys;

TEST_F(KeyVaultClientTest, UpdateProperties)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  auto keyName = GetUniqueName();
  auto updateTo = DateTime::Parse("20301031T00:00:00Z", DateTime::DateFormat::Rfc3339);
  {
    auto keyResponse = keyClient.CreateKey(keyName, JsonWebKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());

    // Update Key
    keyVaultKey.Properties.Enabled = false;
    keyVaultKey.Properties.ExpiresOn = updateTo;
    auto updatedResponse = keyClient.UpdateKeyProperties(keyVaultKey.Properties);
    CheckValidResponse(updatedResponse);
  }
  {
    // Get updated key to check values
    auto updatedKey = keyClient.GetKey(keyName);
    CheckValidResponse(updatedKey);
    auto key = updatedKey.Value;
    EXPECT_TRUE(key.Properties.Enabled);
    EXPECT_FALSE(key.Properties.Enabled.Value());
  }
}

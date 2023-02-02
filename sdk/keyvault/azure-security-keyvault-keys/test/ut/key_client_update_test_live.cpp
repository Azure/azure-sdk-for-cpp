// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/core/datetime.hpp>
#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure;
using namespace Azure::Security::KeyVault::Keys;

TEST_F(KeyVaultKeyClient, UpdateProperties)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  auto updateTo = DateTime::Parse("20301031T00:00:00Z", DateTime::DateFormat::Rfc3339);
  {
    auto keyResponse = client.CreateKey(keyName, KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());

    // Update Key
    keyVaultKey.Properties.Enabled = false;
    keyVaultKey.Properties.ExpiresOn = updateTo;
    auto updatedResponse = client.UpdateKeyProperties(keyVaultKey.Properties);
    CheckValidResponse(updatedResponse);
  }
  {
    // Get updated key to check values
    auto updatedKey = client.GetKey(keyName);
    CheckValidResponse(updatedKey);
    auto key = updatedKey.Value;
    EXPECT_TRUE(key.Properties.Enabled);
    EXPECT_FALSE(key.Properties.Enabled.Value());
  }
}

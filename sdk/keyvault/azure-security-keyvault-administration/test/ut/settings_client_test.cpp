// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "azure/keyvault/administration/settings_client.hpp"
#include "settings_client_base_test.hpp"
#include <azure/core/base64.hpp>
#include <azure/keyvault/administration/rest_client_models.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::Models;
using namespace Azure::Security::KeyVault::Administration::Test;

using namespace std::chrono_literals;

TEST_F(SettingsClientTest, GetSettings)
{
  auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  EXPECT_EQ(testName, testName);
  CreateHSMClientForTest();
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    // create certificate method contains all the checks
    auto const& client = GetClientForTest(testName);
    auto result = client.GetSettings();
    EXPECT_EQ(result.Value.Value.size(), 1);
    auto setting = result.Value.Value[0];
    EXPECT_EQ(setting.Name, "AllowKeyManagementOperationsThroughARM");
    EXPECT_EQ(setting.Value, "false");
  }
}

TEST_F(SettingsClientTest, GetSetting)
{
  auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  CreateHSMClientForTest();
  // create certificate method contains all the checks
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto const& client = GetClientForTest(testName);
    auto result = client.GetSetting("AllowKeyManagementOperationsThroughARM");
    EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
    EXPECT_EQ(result.Value.Value, "false");
  }
}

TEST_F(SettingsClientTest, UpdateSetting)
{
  auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  CreateHSMClientForTest();
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    // create certificate method contains all the checks
    auto const& client = GetClientForTest(testName);
    {
      std::string value = "false";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);

      EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      EXPECT_EQ(result.Value.Value, "false");
    }
    {
      
      std::string value = "true";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);

      EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      EXPECT_EQ(result.Value.Value, "true");
    }
    {
      std::string value = "false";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);

      EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      EXPECT_EQ(result.Value.Value, "false");
    }
  }
}

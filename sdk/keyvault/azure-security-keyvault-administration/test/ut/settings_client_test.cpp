// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/administration/settings_client.hpp"
#include "settings_client_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/administration/rest_client_models.hpp>

#include <cstddef>
#include <string>
#include <thread>

#include <gtest/gtest.h>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::Models;
using namespace Azure::Security::KeyVault::Administration::Test;

using namespace std::chrono_literals;

TEST_F(SettingsClientTest, GetSettings_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "GetSettings";
    EXPECT_EQ(testName, testName);
    CreateHSMClientForTest();

    auto const& client = GetClientForTest(testName);
    auto result = client.GetSettings();
    EXPECT_EQ(result.Value.Value.size(), 1);
    auto setting = result.Value.Value[0];
    if (setting.Name != "Sanitized")
    {
      EXPECT_EQ(setting.Name, "AllowKeyManagementOperationsThroughARM");
    }
    EXPECT_EQ(setting.Value, "false");
  }
  else
  {
    SkipTest();
  }
}

TEST_F(SettingsClientTest, GetSetting_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "GetSetting";
    CreateHSMClientForTest();
    auto const& client = GetClientForTest(testName);
    auto result = client.GetSetting("AllowKeyManagementOperationsThroughARM");
    if (result.Value.Name != "Sanitized")
    {
      EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
    }

    EXPECT_EQ(result.Value.Value, "false");
  }
  else
  {
    SkipTest();
  }
}

TEST_F(SettingsClientTest, UpdateSetting_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "UpdateSetting";
    CreateHSMClientForTest();

    auto const& client = GetClientForTest(testName);
    {
      std::string value = "false";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);
      if (result.Value.Name != "Sanitized")
      {
        EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      }
      EXPECT_EQ(result.Value.Value, "false");
    }
    {

      std::string value = "true";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);
      if (result.Value.Name != "Sanitized")
      {
        EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      }
      EXPECT_EQ(result.Value.Value, "true");
    }
    {
      std::string value = "false";
      auto result = client.UpdateSetting("AllowKeyManagementOperationsThroughARM", value);
      if (result.Value.Name != "Sanitized")
      {
        EXPECT_EQ(result.Value.Name, "AllowKeyManagementOperationsThroughARM");
      }
      EXPECT_EQ(result.Value.Value, "false");
    }
  }
  else
  {
    SkipTest();
  }
}

TEST(KeyVaultSettingsClientTest, ServiceVersion)
{
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>("tenantID", "AppId", "SecretId");
  // Default - 7.4
  EXPECT_NO_THROW(auto options = SettingsClientOptions(); SettingsClient settingsClient(
                      "http://account.vault.azure.net", credential, options);
                  EXPECT_EQ(options.ApiVersion, "7.4"););

  // 7.4
  EXPECT_NO_THROW(
      auto options = SettingsClientOptions(); options.ApiVersion = "7.4";
      SettingsClient settingsClient("http://account.vault.azure.net", credential, options);
      EXPECT_EQ(options.ApiVersion, "7.4"););
}

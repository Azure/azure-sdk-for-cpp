// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "settings_client_base_test.hpp"
#include "azure/keyvault/administration/settings_client.hpp"
#include <azure/core/base64.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace std::chrono_literals;
using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::Test;

using namespace std::chrono_literals;

TEST_F(KeyVaultSettingsClientTest, CreateClient)
{
  auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  EXPECT_EQ(testName, testName);
  // create certificate method contains all the checks
  auto const& client = GetClientForTest(testName);
  auto result = client.GetSettings();
  EXPECT_FALSE(result.Value.value.size() == 0);
}

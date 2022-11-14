//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>

#include <iostream>
#include <string>
#include <thread>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;

TEST_F(KeyVaultKeyClient, BackupKey)
{
  std::string keyName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
  auto const& client = GetClientForTest(keyName);

  std::cout
      << std::endl
      << "Backup key test takes more than 2 minutes since it needs to wait for purge to complete.";

  {
    std::cout << std::endl << "- Create key";
    auto response = client.CreateKey(keyName, KeyVaultKeyType::Ec);
    CheckValidResponse(response);
  }

  // backup
  std::cout << std::endl << "- Backup key";
  auto backUpResponse = client.BackupKey(keyName);
  CheckValidResponse(backUpResponse);
  {
    // Delete
    std::cout << std::endl << "- Delete key";
    auto response = client.StartDeleteKey(keyName);
    response.PollUntilDone(m_testPollingIntervalMs);
  }
  {
    // Purge
    std::cout << std::endl << "- Purge key";
    auto response = client.PurgeDeletedKey(keyName);
    CheckValidResponse(response, Azure::Core::Http::HttpStatusCode::NoContent);
    // Purge can take up to 2 min
    TestSleep(4min);
  }
  { // Check key is gone
    EXPECT_THROW(client.GetKey(keyName), Azure::Core::RequestFailedException);
  }
  {
    // Restore
    std::cout << std::endl << "- Restore key";
    auto respone = client.RestoreKeyBackup(backUpResponse.Value.BackupKey);
    CheckValidResponse(backUpResponse);
  }
  {
    // Check key is restored
    auto response = client.GetKey(keyName);
    CheckValidResponse(response);
    EXPECT_EQ(keyName, response.Value.Name());
  }
}

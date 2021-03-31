// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/details/key_constants.hpp>

#include <iostream>
#include <string>
#include <thread>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;

TEST_F(KeyVaultClientTest, BackupKey)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName = GetUniqueName();

  std::cout
      << std::endl
      << "Backup key test takes more than 2 minutes since it needs to wait for purge to complete.";

  {
    std::cout << std::endl << "- Create key";
    auto response = keyClient.CreateKey(keyName, JsonWebKeyType::Ec);
    CheckValidResponse(response);
  }

  // backup
  std::cout << std::endl << "- Backup key";
  auto backUpResponse = keyClient.BackupKey(keyName);
  CheckValidResponse(backUpResponse);
  {
    // Delete
    std::cout << std::endl << "- Delete key";
    auto response = keyClient.StartDeleteKey(keyName);
    response.PollUntilDone(std::chrono::milliseconds(1000));
  }
  {
    // Purge
    std::cout << std::endl << "- Purge key";
    auto response = keyClient.PurgeDeletedKey(keyName);
    CheckValidResponse(response, Azure::Core::Http::HttpStatusCode::NoContent);
    // Purge can take up to 2 min
    std::this_thread::sleep_for(std::chrono::minutes(2));
  }
  { // Check key is gone
    EXPECT_THROW(keyClient.GetKey(keyName), Azure::Security::KeyVault::Common::KeyVaultException);
  }
  {
    // Restore
    std::cout << std::endl << "- Restore key";
    auto respone = keyClient.RestoreKeyBackup(*backUpResponse);
    CheckValidResponse(backUpResponse);
  }
  {
    // Check key is restored
    auto response = keyClient.GetKey(keyName);
    CheckValidResponse(response);
    EXPECT_EQ(keyName, response->Name());
  }
  {
    // Delete
    std::cout << std::endl << "- Clean";
    auto response = keyClient.StartDeleteKey(keyName);
    response.PollUntilDone(std::chrono::milliseconds(1000));
  }
  {
    // Purge
    auto response = keyClient.PurgeDeletedKey(keyName);
    CheckValidResponse(response, Azure::Core::Http::HttpStatusCode::NoContent);
  }
}

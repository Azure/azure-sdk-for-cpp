// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/administration/backup_restore_client.hpp"
#include "backup_restore_client_base_test.hpp"

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

TEST_F(BackupRestoreClientTest, CreateClient1)
{
  auto& client = GetClientForTest("CreateClient1");
  (void)client;
  client.FullBackup();
  // client.~BackupRestoreClient->FullBackup();
}

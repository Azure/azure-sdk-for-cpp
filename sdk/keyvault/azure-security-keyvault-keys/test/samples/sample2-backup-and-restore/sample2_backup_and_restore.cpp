//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault SDK client for C++
 * to back up and restore a key.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keys.hpp>

#include <assert.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::KeyVault::Keys;

static void AssertKeysEqual(KeyProperties const& expected, KeyProperties const& actual);

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  std::string rsaKeyName("CloudRsaKey" + Azure::Core::Uuid::CreateUuid().ToString());
  try
  {
    auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
    rsaKey.KeySize = 2048;
    rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    std::cout << "\t-Create Key" << std::endl;
    auto storedKey = keyClient.CreateRsaKey(rsaKey).Value;
    size_t backUpSize = 0;
    {
      std::cout << "\t-Backup Key" << std::endl;
      std::vector<uint8_t> backupKey(keyClient.BackupKey(rsaKeyName).Value.BackupKey);
      backUpSize = backupKey.size();

      // save data to file
      std::cout << "\t-Save to file" << std::endl;
      std::ofstream savedFile;
      savedFile.open("backup.dat");
      for (auto const& data : backupKey)
      {
        savedFile << data;
      }
      savedFile.close();
    }
    // backup key is destroy at this point as it is out of the scope.
    // The storage account key is no longer in use, so you delete it.
    std::cout << "\t-Delete and purge key" << std::endl;
    DeleteKeyOperation operation = keyClient.StartDeleteKey(rsaKeyName);
    // You only need to wait for completion if you want to purge or recover the key.
    operation.PollUntilDone(std::chrono::milliseconds(2000));
    keyClient.PurgeDeletedKey(rsaKeyName);
    // let's wait for one minute so we know the key was purged.
    std::this_thread::sleep_for(std::chrono::seconds(60));

    // Restore the key from the file backup
    std::cout << "\t-Read from file." << std::endl;
    std::ifstream inFile;
    inFile.open("backup.dat");
    std::vector<uint8_t> inMemoryBackup(backUpSize);
    inFile >> inMemoryBackup.data();
    inFile.close();

    std::cout << "\t-Restore Key" << std::endl;
    auto restoredKey = keyClient.RestoreKeyBackup(inMemoryBackup).Value;

    AssertKeysEqual(storedKey.Properties, restoredKey.Properties);

    operation = keyClient.StartDeleteKey(rsaKeyName);
    // You only need to wait for completion if you want to purge or recover the key.
    operation.PollUntilDone(std::chrono::milliseconds(2000));
    keyClient.PurgeDeletedKey(rsaKeyName);
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }

  return 0;
}

template <class T>
static inline bool CompareNullableT(Azure::Nullable<T> const& left, Azure::Nullable<T> const& right)
{
  if (!left.HasValue() && !right.HasValue())
  {
    return true;
  }
  if (left.HasValue() && !right.HasValue())
  {
    return false;
  }
  if (!left.HasValue() && right.HasValue())
  {
    return false;
  }
  return left.Value() == right.Value();
}

void AssertKeysEqual(KeyProperties const& expected, KeyProperties const& actual)
{
#if defined(NDEBUG)
  // Use (void) to silent unused warnings.
  (void)expected;
  (void)actual;
#endif
  assert(expected.Name == actual.Name);
  assert(expected.Version == actual.Version);
  assert(expected.Managed == actual.Managed);
  assert(expected.RecoveryLevel == actual.RecoveryLevel);
  assert(CompareNullableT(expected.ExpiresOn, actual.ExpiresOn));
  assert(CompareNullableT(expected.NotBefore, actual.NotBefore));
}

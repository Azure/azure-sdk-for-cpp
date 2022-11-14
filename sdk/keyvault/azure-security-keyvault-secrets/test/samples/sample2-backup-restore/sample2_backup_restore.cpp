//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to backup, restore, delete and purge a secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/identity.hpp>
#include <azure/keyvault/secrets.hpp>

#include <assert.h>
#include <chrono>
#include <fstream>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;
void AssertSecretsEqual(KeyVaultSecret const& expected, KeyVaultSecret const& actual);

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  // create client
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  std::string secretName("MySampleSecret2");
  std::string secretValue("my secret value");

  try
  {
    // create secret
    secretClient.SetSecret(secretName, secretValue);

    // get secret
    KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::cout << "Secret is returned with name " << secret.Name << " and value "
              << secret.Value.Value() << std::endl;

    size_t backUpSize = 0;
    {
      std::cout << "\t-Backup Secret" << std::endl;
      auto backupSecretResult = secretClient.BackupSecret(secret.Name).Value;
      auto const& backedupSecret = backupSecretResult.Secret;
      backUpSize = backedupSecret.size();

      // save data to file
      std::cout << "\t-Save to file" << std::endl;
      std::ofstream savedFile;
      savedFile.open("backup.dat");
      for (auto const& data : backedupSecret)
      {
        savedFile << data;
      }
      savedFile.close();
    }
    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);
    // purge the deleted secret
    secretClient.PurgeDeletedSecret(secret.Name);

    // let's wait for one minute so we know the secret was purged.
    std::this_thread::sleep_for(60s);

    // Restore the secret from the file backup
    std::cout << "\t-Read from file." << std::endl;
    std::ifstream inFile;
    inFile.open("backup.dat");
    BackupSecretResult backedUpSecret;
    backedUpSecret.Secret = std::vector<uint8_t>(backUpSize);
    inFile >> backedUpSecret.Secret.data();
    inFile.close();

    std::cout << "\t-Restore Secret" << std::endl;
    auto restoredSecret = secretClient.RestoreSecretBackup(backedUpSecret).Value;

    AssertSecretsEqual(secret, restoredSecret);

    operation = secretClient.StartDeleteSecret(restoredSecret.Name);
    // You only need to wait for completion if you want to purge or recover the secret.
    operation.PollUntilDone(2s);
    secretClient.PurgeDeletedSecret(restoredSecret.Name);
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Secret Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }

  return 0;
}

void AssertSecretsEqual(KeyVaultSecret const& expected, KeyVaultSecret const& actual)
{
#if defined(NDEBUG)
  // Use (void) to silence unused warnings.
  (void)expected;
  (void)actual;
#endif
  assert(expected.Name == actual.Name);
  assert(expected.Properties.Version == actual.Properties.Version);
  assert(expected.Id == actual.Id);
}

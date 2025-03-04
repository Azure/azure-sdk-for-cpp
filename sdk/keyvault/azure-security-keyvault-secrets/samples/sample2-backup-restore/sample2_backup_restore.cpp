// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to backup, restore, delete and purge a secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/identity.hpp>
#include <azure/security/keyvault/secrets.hpp>

#include <assert.h>
#include <chrono>
#include <fstream>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;
void AssertSecretsEqual(
    Models::KeyVaultSecret const& expected,
    Models::KeyVaultSecret const& actual);

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SecretClient secretClient(keyVaultUrl, credential);

  std::string secretName("MySampleSecret2");
  std::string secretValue("my secret value");

  try
  {
    // create secret
    secretClient.SetSecret(secretName, secretValue);

    // get secret
    Models::KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::string valueString = secret.Value.HasValue() ? secret.Value.Value() : "NONE RETURNED";
    std::cout << "Secret is returned with Id: " << secret.Id.Value() << " and value: " << valueString
              << std::endl;

    size_t backUpSize = 0;
    {
      std::cout << "\t-Backup Secret" << std::endl;
      auto backupSecretResult = secretClient.BackupSecret(secretName).Value;
      auto const& backedupSecret = backupSecretResult.Value.Value();
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
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secretName);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);
    // purge the deleted secret
    secretClient.PurgeDeletedSecret(secretName);

    // let's wait for one minute so we know the secret was purged.
    std::this_thread::sleep_for(60s);

    // Restore the secret from the file backup
    std::cout << "\t-Read from file." << std::endl;
    std::ifstream inFile;
    inFile.open("backup.dat");
    Models::BackupSecretResult backedUpSecret;
    backedUpSecret.Value = std::vector<uint8_t>(backUpSize);
    inFile >> backedUpSecret.Value.Value().data();
    inFile.close();

    std::cout << "\t-Restore Secret" << std::endl;
    auto restoredSecret = secretClient.RestoreSecretBackup(backedUpSecret).Value;

    AssertSecretsEqual(secret, restoredSecret);

    operation = secretClient.StartDeleteSecret(secretName);
    // You only need to wait for completion if you want to purge or recover the secret.
    operation.PollUntilDone(2s);
    secretClient.PurgeDeletedSecret(secretName);
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

void AssertSecretsEqual(
    Models::KeyVaultSecret const& expected,
    Models::KeyVaultSecret const& actual)
{
#if defined(NDEBUG)
  // Use (void) to silence unused warnings.
  (void)expected;
  (void)actual;
#endif
  assert(expected.Id.Value() == actual.Id.Value());
}

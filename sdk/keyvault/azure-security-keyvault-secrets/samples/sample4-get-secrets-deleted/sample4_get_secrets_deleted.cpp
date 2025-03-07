// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to get secrets, get secret versions, get deleted secrets, get deleted secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/identity.hpp>
#include <azure/keyvault/secrets.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  // create client
  SecretClient secretClient(keyVaultUrl, credential);

  std::string secretName("Secret1");
  std::string secretName2("Secret2");
  std::string secretValue("my secret value");

  try
  {
    // create secret
    KeyVaultSecret secret1 = secretClient.SetSecret(secretName, secretValue).Value;
    KeyVaultSecret secret2 = secretClient.SetSecret(secretName2, secretValue).Value;

    std::cout << "Secret1 Id : " << secret1.Id.Value() << std::endl;

    // get properties of secrets
    for (auto secrets = secretClient.GetPropertiesOfSecrets(); secrets.HasPage();
         secrets.MoveToNextPage())
    { // go through every secret of each page returned
      // the number of results returned for in a  page is not guaranteed
      // it can be anywhere from 0 to 25
      for (auto const& secret : secrets.Value.Value())
      {
        std::cout << "Found Secret with Id: " << secret.Id.Value() << std::endl;
      }
    }

    // get all the versions of a secret
    for (auto secretsVersion = secretClient.GetPropertiesOfSecretsVersions(secretName);
         secretsVersion.HasPage();
         secretsVersion.MoveToNextPage())
    { // go through each version of the secret
      // the number of results returned for in a page is not guaranteed
      // it can be anywhere from 0 to 25
      for (auto const& secret : secretsVersion.Value.Value())
      {
        std::cout << "Found Secret with Id: " << secret.Id.Value() << std::endl;
      }
    }

    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secretName);
    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);

    // start deleting the secret
    operation = secretClient.StartDeleteSecret(secretName2);
    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);

    // get all the versions of a secret
    for (auto deletedSecrets = secretClient.GetDeletedSecrets(); deletedSecrets.HasPage();
         deletedSecrets.MoveToNextPage())
    { // the number of results returned for in a  page is not guaranteed
      // it can be anywhere from 0 to 25
      // go through each deleted secret
      for (auto const& deletedSecret : deletedSecrets.Value.Value())
      {
        std::cout << "Found Secret with Id: " << deletedSecret.Id.Value() << std::endl;
      }
    }

    // get one deleted secret
    auto deletedSecret = secretClient.GetDeletedSecret(secretName);
    std::cout << "Deleted Secret with Id: " << deletedSecret.Value.Id.Value();

    // cleanup
    secretClient.PurgeDeletedSecret(secretName);
    secretClient.PurgeDeletedSecret(secretName2);
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

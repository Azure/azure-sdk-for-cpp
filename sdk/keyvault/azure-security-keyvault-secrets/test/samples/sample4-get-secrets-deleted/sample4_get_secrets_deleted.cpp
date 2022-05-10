// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to get secrets, get secret versions, get deleted secrets, get deleted secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL: The Key Vault account URL.
 * - AZURE_TENANT_ID: Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID: The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET or AZURE_CLIENT_CERTIFICATE_PATH: The client secret or certificate path.
 *
 */

#include <get_env.hpp>

#include <azure/identity.hpp>
#include <azure/keyvault/keyvault_secrets.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;

int main()
{
  auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();

  // create client
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  std::string secretName("Secret1");
  std::string secretName2("Secret2");
  std::string secretValue("my secret value");

  try
  {
    // create secret
    KeyVaultSecret secret1 = secretClient.SetSecret(secretName, secretValue).Value;
    KeyVaultSecret secret2 = secretClient.SetSecret(secretName2, secretValue).Value;

    std::cout << "Secret1 Version : " << secret1.Properties.Version << std::endl;

    // get properties of secrets
    for (auto secrets = secretClient.GetPropertiesOfSecrets(); secrets.HasPage();
         secrets.MoveToNextPage())
    { // go through every secret of each page returned
      // the number of results returned for in a  page is not guaranteed
      // it can be anywhere from 0 to 25
      for (auto const& secret : secrets.Items)
      {
        std::cout << "Found Secret with name: " << secret.Name << std::endl;
      }
    }

    // get all the versions of a secret
    for (auto secretsVersion = secretClient.GetPropertiesOfSecretsVersions(secret1.Name);
         secretsVersion.HasPage();
         secretsVersion.MoveToNextPage())
    { // go through each version of the secret
      // the number of results returned for in a  page is not guaranteed
      // it can be anywhere from 0 to 25
      for (auto const& secret : secretsVersion.Items)
      {
        std::cout << "Found Secret with name: " << secret.Name
                  << " and with version: " << secret.Version << std::endl;
      }
    }

    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret1.Name);
    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);

    // start deleting the secret
    operation = secretClient.StartDeleteSecret(secret2.Name);
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
      for (auto const& deletedSecret : deletedSecrets.Items)
      {
        std::cout << "Found Secret with name: " << deletedSecret.Name << std::endl;
      }
    }

    // get one deleted secret
    auto deletedSecret = secretClient.GetDeletedSecret(secret1.Name);
    std::cout << "Deleted Secret with name: " << deletedSecret.Value.Name;

    // cleanup
    secretClient.PurgeDeletedSecret(secret1.Name);
    secretClient.PurgeDeletedSecret(secret2.Name);
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

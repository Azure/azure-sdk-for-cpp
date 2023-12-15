// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to create, get, update, delete and purge a secret.
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
  // @begin_snippet: SecretSample1CreateCredential
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  // @end_snippet

  try
  {
    // create secret
    // @begin_snippet: SecretSample1CreateSecret
    std::string secretName("MySampleSecret");
    std::string secretValue("my secret value");

    secretClient.SetSecret(secretName, secretValue);
    // @end_snippet

    // @begin_snippet: SecretSample1GetSecret
    // get secret
    KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::cout << "Secret is returned with name " << secret.Name << " and value "
              << secret.Value.Value() << std::endl;
    // @end_snippet

    // @begin_snippet: SecretSample1UpdateSecretProperties
    // change one of the properties
    secret.Properties.ContentType = "my content";
    // update the secret
    KeyVaultSecret updatedSecret = secretClient.UpdateSecretProperties(secret.Properties).Value;
    std::cout << "Secret's content type is now " << updatedSecret.Properties.ContentType.Value()
              << std::endl;
    // @end_snippet

    // @begin_snippet: SecretSample1DeleteSecret
    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(20s);

    // purge the deleted secret
    secretClient.PurgeDeletedSecret(secret.Name);
    // @end_snippet
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

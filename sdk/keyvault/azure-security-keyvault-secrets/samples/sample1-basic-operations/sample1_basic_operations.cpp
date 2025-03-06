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

  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SecretClient secretClient(keyVaultUrl, credential);

  try
  {
    // create secret
    // @begin_snippet: SecretSample1CreateSecret
    std::string secretName("MySampleSecret");
    std::string secretValue("my secret value");

    secretClient.SetSecret(secretName, secretValue);
    // @end_snippet

    // get secret
    KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::string valueString = secret.Value.HasValue() ? secret.Value.Value() : "NONE RETURNED";
    std::cout << "Secret is returned with Id " << secret.Id.Value() << " and value " << valueString
              << std::endl;

    // change one of the properties
    UpdateSecretPropertiesOptions options;
    options.ContentType = "my content";
    // update the secret
    KeyVaultSecret updatedSecret = secretClient.UpdateSecretProperties(secretName, options).Value;
    std::string updatedValueString = updatedSecret.ContentType.HasValue()
        ? updatedSecret.ContentType.Value()
        : "NONE RETURNED";
    std::cout << "Secret's content type is now : " << updatedValueString << std::endl;

    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secretName);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(20s);
    std::cout << "Deleted secret with Id " << operation.Value().Id.Value() << std::endl;
    // purge the deleted secret
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

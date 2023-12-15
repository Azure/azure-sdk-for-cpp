// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to delete and restore a secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/identity.hpp>
#include <azure/keyvault/secrets.hpp>

#include <assert.h>
#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;
void AssertSecretsEqual(KeyVaultSecret const& expected, KeyVaultSecret const& actual);

int main()
{
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  std::string secretName("MySampleSecret");
  std::string secretValue("my secret value");

  try
  {

    // create secret
    secretClient.SetSecret(secretName, secretValue);

    // get secret
    KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::cout << "Secret is returned with name " << secret.Name << " and value "
              << secret.Value.Value() << std::endl;

    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);

    // call recover secret
    RecoverDeletedSecretOperation recoverOperation
        = secretClient.StartRecoverDeletedSecret(secret.Name);

    // poll until done
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    SecretProperties restoredSecretProperties = recoverOperation.PollUntilDone(2s).Value;
    KeyVaultSecret restoredSecret = secretClient.GetSecret(restoredSecretProperties.Name).Value;

    AssertSecretsEqual(secret, restoredSecret);

    // cleanup
    // start deleting the secret
    DeleteSecretOperation cleanupOperation = secretClient.StartDeleteSecret(restoredSecret.Name);
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    cleanupOperation.PollUntilDone(2s);
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

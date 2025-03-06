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
void AssertSecretsEqual(
    Models::KeyVaultSecret const& expected,
    Models::KeyVaultSecret const& actual);

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  // create client
  SecretClient secretClient(keyVaultUrl, credential);

  std::string secretName("MySampleSecret");
  std::string secretValue("my secret value");

  try
  {

    // create secret
    secretClient.SetSecret(secretName, secretValue);

    // get secret
    Models::KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;

    std::string valueString = secret.Value.HasValue() ? secret.Value.Value() : "NONE RETURNED";
    std::cout << "Secret is returned with Id: " << secret.Id.Value()
              << " and value: " << valueString << std::endl;
    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secretName);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(2s);

    // call recover secret
    RecoverDeletedSecretOperation recoverOperation
        = secretClient.StartRecoverDeletedSecret(secretName);

    // poll until done
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    Models::KeyVaultSecret restoredSecretProperties = recoverOperation.PollUntilDone(2s).Value;
    Models::KeyVaultSecret restoredSecret = secretClient.GetSecret(secretName).Value;

    AssertSecretsEqual(secret, restoredSecret);

    // cleanup
    // start deleting the secret
    DeleteSecretOperation cleanupOperation = secretClient.StartDeleteSecret(secretName);
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    cleanupOperation.PollUntilDone(2s);
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
  assert(expected.Value.Value() == actual.Value.Value());
}

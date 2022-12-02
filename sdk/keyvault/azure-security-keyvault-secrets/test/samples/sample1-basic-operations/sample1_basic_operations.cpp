// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to create, get, update, delete and purge a secret.
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

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace std::chrono_literals;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

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

    // change one of the properties
    secret.Properties.ContentType = "my content";
    // update the secret
    KeyVaultSecret updatedSecret = secretClient.UpdateSecretProperties(secret.Properties).Value;
    std::cout << "Secret's content type is now " << updatedSecret.Properties.ContentType.Value()
              << std::endl;

    // start deleting the secret
    DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);

    // You only need to wait for completion if you want to purge or recover the secret.
    // The duration of the delete operation might vary
    // in case returns too fast increase the timeout value
    operation.PollUntilDone(20s);

    // purge the deleted secret
    secretClient.PurgeDeletedSecret(secret.Name);
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
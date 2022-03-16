// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides smoke test for the sdks to ensure side by side works properly
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
#include <azure/keyvault/keyvault_certificates.hpp>
#include <azure/keyvault/keyvault_keys.hpp>
#include <azure/keyvault/keyvault_secrets.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Certificates;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  // instantiate the clients
  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  try
  {
    // call an API from each client to make sure all is properly initialized
    keyClient.GetPropertiesOfKeys();
    secretClient.GetPropertiesOfSecrets();
    certificateClient.GetPropertiesOfCertificates();
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }

  return 0;
}

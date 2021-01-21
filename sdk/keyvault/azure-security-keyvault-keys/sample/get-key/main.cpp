// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief The next sample provides the code implementation to use the Key Vault SDK client for C++
 * to create a key client and get a key from Key Vault service.
 *
 * @remark Make sure to set the next environment variables before running the sample.
 * - AZURE_KEYVAULT_URL:           To the KeyVault account url.
 * - AZURE_KEYVAULT_TENANT_ID:     Tenant id for the Azure account.
 * - AZURE_KEYVAULT_CLIENT_ID:     The client id to authenticate the request.
 * - AZURE_KEYVAULT_CLIENT_SECRET: The secret id from the client id.
 *
 * Also, make sure the key is already created. Then set the key name as `KEY_VAULT_KEY_NAME` before
 * the main() method below.
 *
 * @remark The sample has logging enabled and will log the HTTP response into the standard output.
 *
 */

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <azure/core/http/http.hpp>
#include <azure/core/logging/logging.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/common/keyvault_exception.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <iostream>
#include <memory>

using namespace Azure::Security::KeyVault::Keys;

// Define the name of the key to get
#define KEY_VAULT_KEY_NAME "keyName"

int main()
{
  Azure::Core::Logging::SetLogListener(
      [](Azure::Core::Logging::LogClassification const&, std::string const& message) {
        std::cout << message << std::endl;
      });
  Azure::Core::Logging::SetLogClassifications({Azure::Core::Http::LogClassification::Response});

  auto tenantId = std::getenv("AZURE_KEYVAULT_TENANT_ID");
  auto clientId = std::getenv("AZURE_KEYVAULT_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_KEYVAULT_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  try
  {
    auto responseT = keyClient.GetKey(KEY_VAULT_KEY_NAME);
    auto key = responseT.ExtractValue();
    std::cout << "KeyId: " << key.Key.Id << std::endl;
    std::cout << "Operations:" << std::endl;
    for (KeyOperation operation : key.KeyOperations())
    {
      std::cout << " - " << operation.ToString() << std::endl;
    }
  }
  catch (Azure::Core::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
  }
  catch (Azure::Security::KeyVault::Common::KeyVaultException const& e)
  {
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
  }

  return 0;
}

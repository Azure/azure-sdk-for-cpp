// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief The next sample provides the code implementation to use the Key Vault SDK client for C++
 * to create a key client and get a key from Key Vault service.
 *
 * @remark Make sure to set the next environment variables before running the sample.
 * - AZURE_KEYVAULT_URL:           To the KeyVault account url.
 * - AZURE_TENANT_ID:     Tenant id for the Azure account.
 * - AZURE_CLIENT_ID:     The client id to authenticate the request.
 * - AZURE_CLIENT_SECRET: The secret id from the client id.
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

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <chrono>
#include <iostream>
#include <memory>

using namespace Azure::Security::KeyVault::Keys;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  std::string rsaKeyName("CloudRsaKey" + Azure::Core::Uuid::CreateUuid().ToString());
  try
  {
    auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
    rsaKey.KeySize = 2048;
    rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(rsaKey);

    KeyVaultKey cloudRsaKey = keyClient.GetKey(rsaKeyName).ExtractValue();
    std::cout << "Key is returned with name " << cloudRsaKey.Name() << " and type "
              << KeyType::KeyTypeToString(cloudRsaKey.GetKeyType()) << std::endl;

    cloudRsaKey.Properties.ExpiresOn
        = cloudRsaKey.Properties.ExpiresOn.GetValue() + std::chrono::hours(24 * 365);
    KeyVaultKey updatedKey = keyClient.UpdateKeyProperties(cloudRsaKey.Properties).ExtractValue();
    std::cout << "Key's updated expiry time is " << updatedKey.Properties.ExpiresOn->ToString()
              << std::endl;
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
  }
  catch (Azure::Security::KeyVault::Common::KeyVaultException const& e)
  {
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
  }

  return 0;
}

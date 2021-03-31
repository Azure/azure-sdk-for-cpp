// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault SDK client for C++
 * to create, get, update, delete and purge a key.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the KeyVault account url.
 * - AZURE_TENANT_ID:     Tenant id for the Azure account.
 * - AZURE_CLIENT_ID:     The client id to authenticate the request.
 * - AZURE_CLIENT_SECRET: The secret id from the client id.
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
#include <thread>

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

    CreateRsaKeyOptions newRsaKey(rsaKeyName);
    newRsaKey.KeySize = 4096;
    newRsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(newRsaKey);

    DeleteKeyOperation operation = keyClient.StartDeleteKey(rsaKeyName);

    // You only need to wait for completion if you want to purge or recover the key.
    operation.PollUntilDone(std::chrono::milliseconds(2000));

    keyClient.PurgeDeletedKey(rsaKeyName);
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Security::KeyVault::Common::KeyVaultException const& e)
  {
    std::cout << "KeyVault Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }

  return 0;
}

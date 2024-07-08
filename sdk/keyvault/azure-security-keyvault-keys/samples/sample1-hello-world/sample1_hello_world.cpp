// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault SDK client for C++
 * to create, get, update, delete and purge a key.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keys.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::KeyVault::Keys;

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  KeyClient keyClient(keyVaultUrl, credential);

  std::string rsaKeyName("CloudRsaKey" + Azure::Core::Uuid::CreateUuid().ToString());
  try
  {
    auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
    rsaKey.KeySize = 2048;
    rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(rsaKey);

    KeyVaultKey cloudRsaKey = keyClient.GetKey(rsaKeyName).Value;
    std::cout << "Key is returned with name " << cloudRsaKey.Name() << " and type "
              << cloudRsaKey.GetKeyType().ToString() << std::endl;

    cloudRsaKey.Properties.ExpiresOn
        = cloudRsaKey.Properties.ExpiresOn.Value() + std::chrono::hours(24 * 365);
    KeyVaultKey updatedKey = keyClient.UpdateKeyProperties(cloudRsaKey.Properties).Value;
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
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }

  return 0;
}

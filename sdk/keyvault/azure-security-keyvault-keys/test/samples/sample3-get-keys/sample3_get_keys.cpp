//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault SDK client for C++
 * to list keys and versions of a given key, and list deleted keys in a soft-delete enabled Key
 * Vault.
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
#include <azure/keyvault/keys.hpp>

#include <chrono>
#include <fstream>
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

  try
  {
    std::string rsaKeyName("CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString());
    auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
    rsaKey.KeySize = 2048;
    rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    std::string ecKeyName("CloudEcKey-" + Azure::Core::Uuid::CreateUuid().ToString());
    auto ecKey = CreateEcKeyOptions(ecKeyName);
    ecKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    std::cout << "\t-Create Keys" << std::endl;
    keyClient.CreateRsaKey(rsaKey);
    keyClient.CreateEcKey(ecKey);

    std::cout << "\t-List Keys" << std::endl;
    for (auto keys = keyClient.GetPropertiesOfKeys(); keys.HasPage(); keys.MoveToNextPage())
    {
      for (auto const& key : keys.Items)
      {
        if (key.Managed)
        {
          continue;
        }
        auto keyWithType = keyClient.GetKey(key.Name).Value;
        std::cout << "Key is returned with name: " << keyWithType.Name()
                  << " and type: " << keyWithType.GetKeyType().ToString() << std::endl;
      }
    }

    // update key
    CreateRsaKeyOptions newRsaKey(rsaKeyName);
    newRsaKey.KeySize = 4096;
    newRsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(newRsaKey);

    // List key versions
    std::cout << "\t-List Key versions" << std::endl;
    for (auto keyVersions = keyClient.GetPropertiesOfKeyVersions(rsaKeyName); keyVersions.HasPage();
         keyVersions.MoveToNextPage())
    {
      for (auto const& key : keyVersions.Items)
      {
        std::cout << "Key's version: " << key.Version << " with name: " << key.Name << std::endl;
      }
    }

    std::cout << "\t-Delete Keys" << std::endl;
    DeleteKeyOperation rsaOperation = keyClient.StartDeleteKey(rsaKeyName);
    DeleteKeyOperation ecOperation = keyClient.StartDeleteKey(ecKeyName);

    // You only need to wait for completion if you want to purge or recover the key.
    rsaOperation.PollUntilDone(std::chrono::milliseconds(2000));
    ecOperation.PollUntilDone(std::chrono::milliseconds(2000));

    std::cout << "\t-List Deleted Keys" << std::endl;

    // Start getting the first page.
    for (auto keysDeletedPage = keyClient.GetDeletedKeys(); keysDeletedPage.HasPage();
         keysDeletedPage.MoveToNextPage())
    {
      for (auto const& key : keysDeletedPage.Items)
      {
        std::cout << "Deleted key's name: " << key.Name()
                  << ", recovery level: " << key.Properties.RecoveryLevel
                  << " and recovery Id: " << key.RecoveryId << std::endl;
      }
    }

    // If the Key Vault is soft-delete enabled, then for permanent deletion, deleted keys needs to
    // be purged.
    keyClient.PurgeDeletedKey(rsaKeyName);
    keyClient.PurgeDeletedKey(ecKeyName);
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

template <class T>
static inline bool CompareNullableT(Azure::Nullable<T> const& left, Azure::Nullable<T> const& right)
{
  if (!left.HasValue() && !right.HasValue())
  {
    return true;
  }
  if (left.HasValue() && !right.HasValue())
  {
    return false;
  }
  if (!left.HasValue() && right.HasValue())
  {
    return false;
  }
  return left.Value() == right.Value();
}

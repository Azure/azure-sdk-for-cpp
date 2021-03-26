// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief The next sample provides the code implementation to use the Key Vault SDK client for C++
 * to list keys and versions of a given key, and list deleted keys in a soft-delete enabled Key
 * Vault.
 *
 * @remark Make sure to set the next environment variables before running the sample.
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
    for (auto keysSinglePage = keyClient.GetPropertiesOfKeysSinglePage().ExtractValue();;)
    {
      for (auto const& key : keysSinglePage.Items)
      {
        if (key.Managed)
        {
          continue;
        }
        auto keyWithType = keyClient.GetKey(key.Name).ExtractValue();
        std::cout << "Key is returned with name: " << keyWithType.Name()
                  << " and type: " << KeyType::KeyTypeToString(keyWithType.GetKeyType())
                  << std::endl;
      }

      if (!keysSinglePage.ContinuationToken.HasValue())
      {
        // No more pages for the response, break the loop
        break;
      }

      // Get the next page
      GetPropertiesOfKeysSinglePageOptions options;
      options.ContinuationToken = keysSinglePage.ContinuationToken.GetValue();
      keysSinglePage = keyClient.GetPropertiesOfKeysSinglePage(options).ExtractValue();
    }

    // update key
    CreateRsaKeyOptions newRsaKey(rsaKeyName);
    newRsaKey.KeySize = 4096;
    newRsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

    keyClient.CreateRsaKey(newRsaKey);

    // List key versions
    std::cout << "\t-List Key versions" << std::endl;
    for (auto keyVersionsSinglePage
         = keyClient.GetPropertiesOfKeyVersionsSinglePage(rsaKeyName).ExtractValue();
         ;)
    {
      for (auto const& key : keyVersionsSinglePage.Items)
      {
        std::cout << "Key's version: " << key.Version << " with name: " << key.Name << std::endl;
      }

      if (!keyVersionsSinglePage.ContinuationToken.HasValue())
      {
        // No more pages for the response, break the loop
        break;
      }

      // Get the next page
      GetPropertiesOfKeyVersionsSinglePageOptions options;
      options.ContinuationToken = keyVersionsSinglePage.ContinuationToken.GetValue();
      keyVersionsSinglePage
          = keyClient.GetPropertiesOfKeyVersionsSinglePage(rsaKeyName, options).ExtractValue();
    }

    std::cout << "\t-Delete Keys" << std::endl;
    DeleteKeyOperation rsaOperation = keyClient.StartDeleteKey(rsaKeyName);
    DeleteKeyOperation ecOperation = keyClient.StartDeleteKey(ecKeyName);

    // You only need to wait for completion if you want to purge or recover the key.
    rsaOperation.PollUntilDone(std::chrono::milliseconds(2000));
    ecOperation.PollUntilDone(std::chrono::milliseconds(2000));

    std::cout << "\t-List Deleted Keys" << std::endl;

    // Start getting the first page.
    for (auto keysDeletedPage = keyClient.GetDeletedKeysSinglePage().ExtractValue();;)
    {
      for (auto const& key : keysDeletedPage.Items)
      {
        std::cout << "Deleted key's name: " << key.Name()
                  << ", recovery level: " << key.Properties.RecoveryLevel
                  << " and recovery Id: " << key.RecoveryId << std::endl;
      }

      if (!keysDeletedPage.ContinuationToken.HasValue())
      {
        // No more pages for the response, break the loop
        break;
      }

      // Get the next page
      GetDeletedKeysSinglePageOptions options;
      options.ContinuationToken = keysDeletedPage.ContinuationToken.GetValue();
      keysDeletedPage = keyClient.GetDeletedKeysSinglePage(options).ExtractValue();
    }

    // If the keyvault is soft-delete enabled, then for permanent deletion, deleted keys needs to be
    // purged.
    keyClient.PurgeDeletedKey(rsaKeyName);
    keyClient.PurgeDeletedKey(ecKeyName);
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
  return left.GetValue() == right.GetValue();
}

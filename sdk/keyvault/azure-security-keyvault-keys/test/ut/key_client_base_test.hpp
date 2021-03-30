// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <gtest/gtest.h>

#include <azure/core/context.hpp>
#include <azure/core/uuid.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <cstdio>
#include <iostream>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  class KeyVaultClientTest : public ::testing::Test {
  protected:
    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;
    std::unique_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_client;

    // Create
    virtual void SetUp() override
    {
      std::string tenantId = std::getenv("AZURE_TENANT_ID");
      std::string clientId = std::getenv("AZURE_CLIENT_ID");
      std::string secretId = std::getenv("AZURE_CLIENT_SECRET");
      m_credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, secretId);

      m_keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
      m_keyVaultHsmUrl = std::getenv("AZURE_KEYVAULT_HSM_URL");
    }

  public:
    template <class T>
    static inline void CheckValidResponse(
        Azure::Response<T>& response,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
    {
      auto const& rawResponse = response.GetRawResponse();
      EXPECT_EQ(
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              rawResponse.GetStatusCode()),
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              expectedCode));
    }

    static inline std::string GetUniqueName() { return Azure::Core::Uuid::CreateUuid().ToString(); }

    static inline void CleanUpKeyVault(KeyClient const& keyClient)
    {
      std::vector<DeletedKey> deletedKeys;
      GetDeletedKeysSinglePageOptions options;
      while (true)
      {
        auto keyResponse = keyClient.GetDeletedKeysSinglePage(options);
        for (auto& key : keyResponse->Items)
        {
          deletedKeys.emplace_back(key);
        }
        if (!keyResponse->ContinuationToken)
        {
          break;
        }
        options.ContinuationToken = keyResponse->ContinuationToken;
      }
      if (deletedKeys.size() > 0)
      {
        for (auto& deletedKey : deletedKeys)
        {
          keyClient.PurgeDeletedKey(deletedKey.Name());
        }
        // Wait for purge is completed
        std::this_thread::sleep_for(std::chrono::minutes(1));
      }
    }

    static inline void RemoveAllKeysFromVault(KeyClient const& keyClient, bool waitForPurge = true)
    {
      std::vector<DeleteKeyOperation> deletedKeys;
      GetPropertiesOfKeysSinglePageOptions options;
      while (true)
      {
        auto keyResponse = keyClient.GetPropertiesOfKeysSinglePage(options);
        for (auto& key : keyResponse->Items)
        {
          deletedKeys.emplace_back(keyClient.StartDeleteKey(key.Name));
        }
        if (!keyResponse->ContinuationToken)
        {
          break;
        }
        options.ContinuationToken = keyResponse->ContinuationToken;
      }
      if (deletedKeys.size() > 0)
      {
        std::cout << std::endl
                  << "Cleaning vault. " << deletedKeys.size()
                  << " Will be deleted and purged now...";
        for (auto& deletedKey : deletedKeys)
        {
          auto readyToPurgeKey = deletedKey.PollUntilDone(std::chrono::milliseconds(1000));
          keyClient.PurgeDeletedKey(readyToPurgeKey->Name());
          std::cout << std::endl << "Deleted and purged key: " + readyToPurgeKey->Name();
        }
        std::cout << std::endl << "Complete purge operation.";
        // Wait for purge is completed
        if (waitForPurge)
        {
          std::this_thread::sleep_for(std::chrono::minutes(1));
        }
      }
    }
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Test

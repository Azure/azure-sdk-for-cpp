// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */
#include <gtest/gtest.h>

#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/secrets.hpp>

using namespace std::chrono_literals;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {

  class KeyVaultSecretClientTest : public Azure::Core::Test::TestBase,
                                   public ::testing::WithParamInterface<int> {
  private:
    std::unique_ptr<Azure::Security::KeyVault::Secrets::SecretClient> m_client;

  protected:
    std::chrono::minutes m_testPollingIntervalMs = std::chrono::minutes(1);
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_keyVaultUrl;
    std::chrono::milliseconds m_defaultWait = 1min;

    Azure::Security::KeyVault::Secrets::SecretClient const& GetClientForTest(
        std::string const& testName)
    {
      // Azure::Core::_internal::Environment::SetVariable("AZURE_TEST_MODE", "PLAYBACK");
      // keep this here to quickly switch between test modes
      InitializeClient();
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    // Create
    void InitializeClient()
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");

      // Options and credential for the client
      SecretClientOptions options;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      // `InitTestClient` takes care of setting up Record&Playback.
      m_client = InitTestClient<
          Azure::Security::KeyVault::Secrets::SecretClient,
          Azure::Security::KeyVault::Secrets::SecretClientOptions>(
          m_keyVaultUrl, m_credential, options);

      // Update default time depending on test mode.
      UpdateWaitingTime(m_defaultWait);
    }

  public:
    template <class T>
    static inline void CheckValidResponse(
        Azure::Response<T>& response,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
    {
      auto const& rawResponse = response.RawResponse;
      EXPECT_EQ(
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              rawResponse->GetStatusCode()),
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              expectedCode));
    }

    static inline std::string GetUniqueName() { return Azure::Core::Uuid::CreateUuid().ToString(); }

    static inline void CleanUpKeyVault(SecretClient const& secretClient)
    {

      std::vector<DeletedSecret> deletedSecrets;
      for (auto secretResponse = secretClient.GetDeletedSecrets(); secretResponse.HasPage();
           secretResponse.MoveToNextPage())
      {
        for (auto& secret : secretResponse.Items)
        {
          deletedSecrets.emplace_back(secret);
        }
      }
      if (deletedSecrets.size() > 0)
      {
        for (auto& deletedSecret : deletedSecrets)
        {
          secretClient.PurgeDeletedSecret(deletedSecret.Name);
        }
        // Wait for purge is completed
        std::this_thread::sleep_for(std::chrono::minutes(1));
      }
    }

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }

    static inline void RemoveAllSecretsFromVault(
        SecretClient const& secretClient,
        bool waitForPurge = true)
    {
      std::vector<DeleteSecretOperation> deletedSecrets;
      GetPropertiesOfSecretsOptions options;
      for (auto secretResponse = secretClient.GetPropertiesOfSecrets(); secretResponse.HasPage();
           secretResponse.MoveToNextPage())
      {
        for (auto& secret : secretResponse.Items)
        {
          deletedSecrets.emplace_back(secretClient.StartDeleteSecret(secret.Name));
        }
      }
      if (deletedSecrets.size() > 0)
      {
        std::cout << std::endl
                  << "Cleaning vault. " << deletedSecrets.size()
                  << " Will be deleted and purged now...";
        for (auto& deletedSecret : deletedSecrets)
        {
          auto readyToPurgeSecret = deletedSecret.PollUntilDone(std::chrono::minutes(1));
          secretClient.PurgeDeletedSecret(readyToPurgeSecret.Value.Name);
          std::cout << std::endl << "Deleted and purged secret: " + readyToPurgeSecret.Value.Name;
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

}}}}} // namespace Azure::Security::KeyVault::Secrets::_test
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */
#include <gtest/gtest.h>

#include <azure/keyvault/keyvault_secrets.hpp>
#include <azure/core/test/test_base.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {

  class KeyVaultSecretClientTest : public Azure::Core::Test::TestBase,
                                   public ::testing::WithParamInterface<int> {
  private:
    std::unique_ptr<Azure::Security::KeyVault::Secrets::SecretClient> m_client;
    std::string GetEnv(const std::string& name, std::string const& defaultValue = std::string())
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
      const char* ret = std::getenv(name.data());
#pragma warning(pop)
#elif
      const char* ret = std::getenv(name.data());
#endif
      
      if (!ret)
      {
        if (defaultValue.size() > 0)
        {
          return defaultValue;
        }

        throw std::runtime_error(
            name + " is required to run the tests but not set as an environment variable.");
      }

      return std::string(ret);
    }

  protected:
    int m_testPollingTimeOutMinutes = 20;
    std::chrono::minutes m_testPollingIntervalMinutes = std::chrono::minutes(1);

    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;

    Azure::Security::KeyVault::Secrets::SecretClient const& GetClientForTest(
        std::string const& testName,
        Azure::Core::Test::TestMode const& testMode)
    {
      std::string testModeValue = "LIVE";

      switch (testMode)
      {
        case Azure::Core::Test::TestMode::RECORD: {
          testModeValue = "RECORD";
        }
        break;
        case Azure::Core::Test::TestMode::PLAYBACK: {
          testModeValue = "PLAYBACK";
        }
        break;
        case Azure::Core::Test::TestMode::LIVE:
        default: {
          testModeValue = "LIVE";
        }
        break;
      };

#if defined(_MSC_VER)
      _putenv_s("AZURE_TEST_MODE", testModeValue.c_str());
#elif
      std::setenv("AZURE_TEST_MODE", testModeValue.c_str(), 1);
#endif
      InitializeClient();
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    // Create
    void InitializeClient()
    {
      // Init interceptor from PlayBackRecorder
      std::string recordingPath(AZURE_TEST_RECORDING_DIR);
      recordingPath.append("/recordings");
      Azure::Core::Test::TestBase::SetUpBase(recordingPath);

      std::string tenantId = GetEnv("AZURE_TENANT_ID");
      std::string clientId = GetEnv("AZURE_CLIENT_ID");
      std::string secretId = GetEnv("AZURE_CLIENT_SECRET");
      m_credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, secretId);

      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");
      m_keyVaultHsmUrl = GetEnv("AZURE_KEYVAULT_HSM_URL");

      // Create default client for the test
      SecretClientOptions options;
      // Replace default transport adapter for playback
      if (m_testContext.IsPlaybackMode())
      {
        options.Transport.Transport = m_interceptor->GetPlaybackClient();
      }
      // Insert Recording policy when Record mode is on (non playback and non LiveMode)
      else if (!m_testContext.IsLiveMode())
      {
        // AZURE_TEST_RECORDING_DIR is exported by CMAKE
        options.PerRetryPolicies.push_back(m_interceptor->GetRecordPolicy());
      }

      m_client = std::make_unique<SecretClient>(m_keyVaultUrl, m_credential, options);

      // When running live tests, service can return 429 error response if the client is sending
      // multiple requests per second. This can happen if the network is fast and tests are running
      // without any delay between them.
      auto avoidTestThrottled = GetEnv("AZURE_KEYVAULT_AVOID_THROTTLED", "0");

      if (avoidTestThrottled != "0")
      {
        std::cout << "- Wait to avoid server throttled..." << std::endl;
        // 10 sec should be enough to prevent from 429 error
        std::this_thread::sleep_for(std::chrono::seconds(10));
      }
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

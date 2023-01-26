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
#include <azure/keyvault/administration.hpp>
#include <chrono>
#include <thread>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Administration {
        namespace Test {

  class SettingsClientTest : public Azure::Core::Test::TestBase,
                             public ::testing::WithParamInterface<int> {

  private:
    std::unique_ptr<Azure::Security::KeyVault::Administration::SettingsClient> m_client;

  protected:
    std::shared_ptr<Core::Credentials::TokenCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;
    std::chrono::milliseconds m_defaultWait = 20s;

    // Required to rename the test propertly once the test is started.
    // We can only know the test instance name until the test instance is run.
    Azure::Security::KeyVault::Administration::SettingsClient const& GetClientForTest(
        std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }
    void CreateHSMClientForTest(std::string hsmUrl = "")
    {
      SettingsClientOptions options;
      m_client = InitTestClient<
          Azure::Security::KeyVault::Administration::SettingsClient,
          Azure::Security::KeyVault::Administration::SettingsClientOptions>(
          hsmUrl.length() == 0 ? m_keyVaultHsmUrl : hsmUrl, m_credential, options);
    }
    // Runs before every test.
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");
      m_keyVaultHsmUrl = GetEnv("AZURE_KEYVAULT_HSM_URL");
      // Options and credential for the client
      SettingsClientOptions options;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      // `InitTestClient` takes care of setting up Record&Playback.
      m_client = InitTestClient<
          Azure::Security::KeyVault::Administration::SettingsClient,
          Azure::Security::KeyVault::Administration::SettingsClientOptions>(
          m_keyVaultUrl, m_credential, options);

      // Update default time depending on test mode.
      UpdateWaitingTime(m_defaultWait);
    }

  public:
    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }

    SettingsClientTest() {m_proxyAssetsPath = AZURE_TEST_ASSETS_DIR; }
  };
}}}}} // namespace Azure::Security::KeyVault::Administration::Test

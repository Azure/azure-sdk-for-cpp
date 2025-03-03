// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/security/keyvault/secrets.hpp>

#include <gtest/gtest.h>

using namespace std::chrono_literals;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {

  class KeyVaultSecretClientTest : public Azure::Core::Test::TestBase,
                                   public ::testing::WithParamInterface<int> {
  public:
    KeyVaultSecretClientTest() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }

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
      m_credential = GetTestCredential();

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

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_test

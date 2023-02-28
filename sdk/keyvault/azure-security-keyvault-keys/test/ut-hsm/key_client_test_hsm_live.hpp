// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */
#include <gtest/gtest.h>

#include <azure/core/context.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/core/uuid.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/keys.hpp>
#include <azure/keyvault/keys/cryptography/cryptography_client.hpp>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  class KeyVaultKeyHSMClient : public Azure::Core::Test::TestBase {
  public:
    KeyVaultKeyHSMClient() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }

  private:
    std::unique_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_client;

  protected:
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;
    int m_testPollingTimeOutMinutes = 20;
    std::chrono::milliseconds m_testPollingIntervalMs = std::chrono::minutes(1);

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating. This also return the name with suffix if the "AZURE_LIVE_TEST_SUFFIX" exists.
    std::string GetTestName(bool sanitize = true)
    {
      auto output = m_keyVaultUrl.compare(m_keyVaultHsmUrl) == 0 ? "Same" : "NotSame";
      std::cout << "\n Keyvault and HSM are" << output;
      return Azure::Core::Test::TestBase::GetTestNameSuffix(sanitize);
    }

    Azure::Security::KeyVault::Keys::KeyClient const& GetClientForTest(std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    // Create
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      m_keyVaultUrl = GetEnv("AZURE_KEYVAULT_URL");
      m_keyVaultHsmUrl = GetEnv("AZURE_KEYVAULT_HSM_URL");

      // Options and credential for the client
      KeyClientOptions options;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

      // `InitTestClient` takes care of setting up Record&Playback.
      m_client = InitTestClient<
          Azure::Security::KeyVault::Keys::KeyClient,
          Azure::Security::KeyVault::Keys::KeyClientOptions>(m_keyVaultUrl, m_credential, options);

      UpdateWaitingTime(m_testPollingIntervalMs);
    }

    void CreateHsmClient(std::string hsmUrl = "")
    {
      KeyClientOptions options;
      m_client = InitTestClient<
          Azure::Security::KeyVault::Keys::KeyClient,
          Azure::Security::KeyVault::Keys::KeyClientOptions>(
          hsmUrl.length() == 0 ? m_keyVaultHsmUrl : hsmUrl, m_credential, options);
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

  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Test

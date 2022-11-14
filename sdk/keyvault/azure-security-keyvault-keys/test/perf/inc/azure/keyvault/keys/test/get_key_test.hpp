//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of getting a key.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keys.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Core::_internal;
namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  /**
   * @brief A test to measure getting a key performance.
   *
   */
  class GetKey : public Azure::Perf::PerfTest {
  private:
    std::string m_vaultUrl;
    std::string m_keyName;
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_secret;
    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::unique_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_client;

  public:
    /**
     * @brief Get the Ids and secret
     *
     */
    void Setup() override
    {
      m_vaultUrl = m_options.GetOptionOrDefault<std::string>(
          "vaultUrl", Environment::GetVariable("AZURE_KEYVAULT_URL"));
      m_tenantId = m_options.GetOptionOrDefault<std::string>(
          "TenantId", Environment::GetVariable("AZURE_TENANT_ID"));
      m_clientId = m_options.GetOptionOrDefault<std::string>(
          "ClientId", Environment::GetVariable("AZURE_CLIENT_ID"));
      m_secret = m_options.GetOptionOrDefault<std::string>(
          "Secret", Environment::GetVariable("AZURE_CLIENT_SECRET"));
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          m_tenantId, m_clientId, m_secret);
      m_client = std::make_unique<Azure::Security::KeyVault::Keys::KeyClient>(
          m_vaultUrl,
          m_credential,
          InitClientOptions<Azure::Security::KeyVault::Keys::KeyClientOptions>());
      this->CreateRandomNameKey();
    }

    /**
     * @brief Create a random named certificate.
     *
     */
    void CreateRandomNameKey()
    {
      std::string name("perf");
      int suffixLen = 10;
      static const char alphanum[]
          = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
      std::string suffix;
      suffix.reserve(suffixLen);

      for (int i = 0; i < suffixLen; ++i)
      {
        suffix += alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      m_keyName = name + suffix;
      auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(m_keyName);
      auto keyResponse = m_client->CreateEcKey(ecKey);
    }

    /**
     * @brief Construct a new GetKey test.
     *
     * @param options The test options.
     */
    GetKey(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override { auto t = m_client->GetKey(m_keyName); }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"vaultUrl", {"--vaultUrl"}, "The Key Vault Account.", 1, false},
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, false},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, false},
          {"Secret", {"--secret"}, "The secret for authentication.", 1, false, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"GetKey", "Get a key", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Security::KeyVault::Keys::Test::GetKey>(options);
              }};
    }
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Test

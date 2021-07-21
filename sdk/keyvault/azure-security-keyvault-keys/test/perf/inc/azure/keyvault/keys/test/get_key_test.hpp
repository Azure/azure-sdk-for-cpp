// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of getting a key.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/identity.hpp>
#include <azure/keyvault/keyvault_keys.hpp>

#include <memory>
#include <string>
#include <vector>

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
      m_vaultUrl = m_options.GetMandatoryOption<std::string>("vaultUrl");
      m_keyName = m_options.GetMandatoryOption<std::string>("keyName");
      m_tenantId = m_options.GetMandatoryOption<std::string>("TenantId");
      m_clientId = m_options.GetMandatoryOption<std::string>("ClientId");
      m_secret = m_options.GetMandatoryOption<std::string>("Secret");
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          m_tenantId, m_clientId, m_secret);
      m_client
          = std::make_unique<Azure::Security::KeyVault::Keys::KeyClient>(m_vaultUrl, m_credential);
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
          {"vaultUrl", {"--vaultUrl"}, "The Key Vault Account.", 1, true},
          {"keyName", {"--keyName"}, "The Key name to get.", 1, true},
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, true},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, true},
          {"Secret", {"--secret"}, "The secret for authentication.", 1, true, true}};
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

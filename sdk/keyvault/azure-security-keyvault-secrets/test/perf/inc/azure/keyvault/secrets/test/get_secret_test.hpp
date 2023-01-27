//   Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of getting a secret.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/secrets.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Core::_internal;
namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace Test {

  /**
   * @brief A test to measure getting a secret performance.
   *
   */
  class GetSecret : public Azure::Perf::PerfTest {
  private:
    std::string m_vaultUrl;
    std::string m_secretName;
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_secret;
    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::unique_ptr<Azure::Security::KeyVault::Secrets::SecretClient> m_client;

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
      m_client = std::make_unique<Azure::Security::KeyVault::Secrets::SecretClient>(
          m_vaultUrl,
          m_credential,
          InitClientOptions<Azure::Security::KeyVault::Secrets::SecretClientOptions>());
      this->CreateRandomNameKey();
    }

    /**
     * @brief Create a random named secret.
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

      m_secretName = name + suffix;

      auto secretResponse = m_client->SetSecret(m_secretName, "secretValue");
    }

    /**
     * @brief Construct a new GetSecret test.
     *
     * @param options The test options.
     */
    GetSecret(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override { auto t = m_client->GetSecret(m_secretName); }

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
      return {"GetSecret", "Get a secret", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Security::KeyVault::Secrets::Test::GetSecret>(
                    options);
              }};
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::Test

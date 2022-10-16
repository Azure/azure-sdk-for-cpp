// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of authenticating with secret credential.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/identity.hpp>

#include "private/token_cache.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Identity { namespace Test {

  /**
   * @brief A test to measure the authentication token performance.
   *
   */
  class SecretCredentialTest : public Azure::Perf::PerfTest {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_secret;
    Core::Credentials::TokenRequestContext m_tokenRequestContext;
    std::unique_ptr<Azure::Identity::ClientSecretCredential> m_credential;

  public:
    /**
     * @brief Get the Id and secret
     *
     */
    void Setup() override
    {
      m_tenantId = m_options.GetMandatoryOption<std::string>("TenantId");
      m_clientId = m_options.GetMandatoryOption<std::string>("ClientId");
      m_secret = m_options.GetMandatoryOption<std::string>("Secret");
      m_tokenRequestContext.Scopes.push_back(m_options.GetMandatoryOption<std::string>("Scope"));
      m_credential = std::make_unique<Azure::Identity::ClientSecretCredential>(
          m_tenantId,
          m_clientId,
          m_secret,
          InitClientOptions<Azure::Core::Credentials::TokenCredentialOptions>());
    }

    /**
     * @brief Construct a new SecretCredentialTest test.
     *
     * @param options The test options.
     */
    SecretCredentialTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     * @param context The cancellation token.
     */
    void Run(Azure::Core::Context const& context) override
    {
      _detail::TokenCache::Clear();
      auto t = m_credential->GetToken(m_tokenRequestContext, context);
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, true},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, true},
          {"Secret", {"--secret"}, "The secret for authentication.", 1, true, true},
          {"Scope", {"--scope"}, "One scope to request access to.", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "SecretCredential",
          "Get a token using a secret client token credential.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Identity::Test::SecretCredentialTest>(options);
          }};
    }
  };

}}} // namespace Azure::Identity::Test

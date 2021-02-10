// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the overhead of authenticating with secret credential.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <azure/identity.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Identity { namespace Test { namespace Performance {

  /**
   * @brief A test to measure the authentication token performance.
   *
   */
  class SecretCredentialTest : public Azure::PerformanceStress::PerformanceTest {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_secret;
    Azure::Core::Http::TokenRequestOptions m_scopes;
    std::unique_ptr<Azure::Identity::ClientSecretCredential> m_credentail;

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
      m_scopes.Scopes.push_back(m_options.GetMandatoryOption<std::string>("Scope"));
      m_credentail = std::make_unique<Azure::Identity::ClientSecretCredential>(
          m_tenantId, m_clientId, m_secret);
    }

    /**
     * @brief Construct a new SecretCredentialTest test.
     *
     * @param options The test options.
     */
    SecretCredentialTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options)
    {
    }

    /**
     * @brief Define the test
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const& context) override
    {
      auto t = m_credentail->GetToken(context, m_scopes);
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
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
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "SecretCredential",
          "Get a token using a secret client token credential.",
          [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::Identity::Test::Performance::SecretCredentialTest>(
                options);
          }};
    }
  };

}}}} // namespace Azure::Identity::Test::Performance

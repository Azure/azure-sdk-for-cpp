// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the overhead of authenticating with secret credential.
 *
 */

#pragma once

#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/perf.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Identity { namespace Test {

  /**
   * @brief A test to measure the authentication token performance.
   *
   */
  class EnvironmentCredentialTest : public Azure::Perf::PerfTest {
  private:
    Core::Credentials::TokenRequestContext m_tokenRequestContext;
    std::unique_ptr<const Azure::Core::Credentials::TokenCredential> m_credential;

  public:
    /**
     * @brief Setup the test
     *
     */
    void Setup() override
    {
      m_tokenRequestContext.Scopes.push_back(
          m_options.GetOptionOrDefault<std::string>("Scope", "https://attest.azure.net/.default"));
      if (!m_options.GetOptionOrDefault<bool>("Cache", false))
      {
        // having this set ignores the credentials cache and forces a new token to be requested
        m_tokenRequestContext.MinimumExpiration = std::chrono::hours(1000000);
      }
      m_credential = std::make_unique<Azure::Identity::EnvironmentCredential>(
          InitClientOptions<Azure::Core::Credentials::TokenCredentialOptions>());
    }

    /**
     * @brief Construct a new EnvironmentCredentialTest test.
     *
     * @param options The test options.
     */
    EnvironmentCredentialTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     * @param context The cancellation token.
     */
    void Run(Azure::Core::Context const& context) override
    {
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
          {"Cache", {"--cache"}, "Use credential cache.", 1, false},
          {"Scope", {"--scope"}, "One scope to request access to.", 1, false}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "EnvironmentCredential",
          "Get a token using a secret client token credential.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Identity::Test::EnvironmentCredentialTest>(options);
          }};
    }
  };

}}} // namespace Azure::Identity::Test

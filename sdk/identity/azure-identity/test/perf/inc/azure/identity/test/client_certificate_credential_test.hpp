// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the overhead of authenticating with client certificate credential.
 *
 */

#pragma once

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
  class ClientCertificateCredentialTest : public Azure::Perf::PerfTest {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_certPath;
    Core::Credentials::TokenRequestContext m_tokenRequestContext;
    std::unique_ptr<const Azure::Core::Credentials::TokenCredential> m_credential;

  public:
    /**
     * @brief Setup the test.
     *
     */
    void Setup() override
    {
      m_tenantId = m_options.GetMandatoryOption<std::string>("TenantId");
      m_clientId = m_options.GetMandatoryOption<std::string>("ClientId");
      m_certPath = m_options.GetMandatoryOption<std::string>("CertPath");
      m_tokenRequestContext.Scopes.push_back(m_options.GetMandatoryOption<std::string>("Scope"));
      if (!m_options.GetOptionOrDefault<bool>("Cache", false))
      {
        // having this set ignores the credentials cache and forces a new token to be requested
        m_tokenRequestContext.MinimumExpiration = std::chrono::hours(1000000);
      }
      m_credential = std::make_unique<Azure::Identity::ClientCertificateCredential>(
          m_tenantId,
          m_clientId,
          m_certPath,
          InitClientOptions<Azure::Core::Credentials::TokenCredentialOptions>());
    }

    /**
     * @brief Construct a new ClientCertificateCredentialTest test.
     *
     * @param options The test options.
     */
    ClientCertificateCredentialTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

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
          {"CertPath", {"--certpath"}, "The certificate path for authentication.", 1, true, true},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, true},
          {"Scope", {"--scope"}, "One scope to request access to.", 1, true},
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "ClientCertificateCredential",
          "Get a token using a client certificate credential.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Identity::Test::ClientCertificateCredentialTest>(
                options);
          }};
    }
  };

}}} // namespace Azure::Identity::Test

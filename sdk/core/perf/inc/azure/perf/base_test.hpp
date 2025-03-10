// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Define the interface of a performance test.
 *
 */

#pragma once

#include "azure/perf/options.hpp"
#include "azure/perf/test_options.hpp"

#include <azure/core/context.hpp>
#include <azure/core/internal/client_options.hpp>
#include <azure/core/url.hpp>

#include <string>
#include <vector>

namespace {
class ProxyPolicy;
}

namespace Azure { namespace Perf {
  class Program;

  /**
   * @brief The base interface for a performance test.
   *
   */
  class BaseTest {
    // Provides private access so a test program can run PostSetup.
    friend class Program;
    friend class ::ProxyPolicy;

  private:
    std::string m_recordId;
    std::string m_proxy;
    bool m_isPlayBackMode = false;
    bool m_isInsecureEnabled = false;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> m_testCredential;

    /**
     * @brief Updates the performance test to use a test-proxy for running.
     *
     * @note A tes-proxy is not a general proxy in the middle of the test and a server. This is an
     * SDK specific tool https://github.com/Azure/azure-sdk-tools/tree/main/tools/test-proxy that
     * provides record and playback features to a performance test. Do not use a general purpose
     * proxy for the test.
     *
     * @param proxy A test-proxy server url.
     */
    void SetTestProxy(std::string const& proxy)
    {
      if (!proxy.empty())
      {
        // we need to allow for certificate verification failures due to presence of test proxy.
        m_isInsecureEnabled = true;
        m_proxy = proxy;
      }
    }

    /**
     * @brief Set the performance test to run insecure.
     *
     * @details Running insecure means that for an SSL connection, the server certificate won't be
     * validated to be a known certificate. Use this to stablish conversation with Https servers
     * using self-signed certificates.
     *
     * @param value Boolean value use to set the insecure mode ON of OFF.
     */
    void AllowInsecureConnections(bool value) { m_isInsecureEnabled = value; }

    /**
     * @brief Define actions to run after test set up and before the actual test.
     *
     * @details This method enables the performance framework to set the proxy server for recordings
     * or any other configuration to happen after a test set up definition.
     *
     */
    void PostSetUp();

    /**
     * @brief Define actions to run after each test run.
     *
     * @details This method enabled the performance framework to remove test proxy forwarding before
     * letting test do clean up.
     *
     */
    void PreCleanUp();

    void ConfigureInsecureConnection(Azure::Core::_internal::ClientOptions& clientOptions);

    /**
     * @brief Utility function used by tests to retrieve env vars
     *
     * @param name Environment variable name to retrieve.
     *
     * @return The value of the environment variable retrieved.
     *
     * @note If AZURE_TENANT_ID or AZURE_CLIENT_ID are not available in the
     * environment, the AZURE_SERVICE_DIRECTORY environment variable is used to set those values
     * with the values emitted by the New-TestResources.ps1 script.
     *
     * @note The Azure CI pipeline upper cases all environment variables defined in the pipeline.
     * Since some operating systems have case sensitive environment variables, on debug builds,
     * this function ensures that the environment variable being retrieved is all upper case.
     *
     */
    std::string GetEnv(std::string const& name);

  protected:
    Azure::Perf::TestOptions m_options;

    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> GetTestCredential();

  public:
    BaseTest(Azure::Perf::TestOptions options) : m_options(options) {}

    /**
     * @brief Run one time at the beginning and before any test.
     *
     * @remark No matter if the parallel option is set to more than one, the global setup will run
     * only once.
     *
     */
    virtual void GlobalSetup() {}

    /**
     * @brief Run one time per each test thread.
     *
     * @remark Each test thread will run the main test in a loop after running the setup from each
     * thread.
     *
     */
    virtual void Setup() {}

    /**
     * @brief Defines the test specific options.
     *
     * @remark The performance framework will parse the test options from the command line. The test
     * can then call `m_options.GetOptionOrDefault(optionName, defaultValue)` to get the value or
     * set a default value.
     *
     * @return The array of test options supported by a performance test.
     */
    virtual std::vector<Azure::Perf::TestOption> GetTestOptions()
    {
      return {
          {"extraOption",
           {"--e"},
           "Example for extended option for test.Needed to be run by the perf tool.",
           1}};
    }

    /**
     * @brief Define the main test case.
     *
     * @remark The test will run over and over in a loop until the duration of the test is reached.
     *
     * @param cancellationToken A cancellation mechanism for the .
     */
    virtual void Run(Azure::Core::Context const& cancellationToken) = 0;

    /**
     * @brief Run one per test thread once that the main test loop finishes.
     *
     * @remark The clean up can be skipped by setting the option `NoCleanup`.
     */
    virtual void Cleanup() {}

    /**
     * @brief Run only once before the test application ends.
     *
     */
    virtual void GlobalCleanup() {}

    /**
     * @brief Set the client options depending on the test options.
     *
     * @param clientOptions ref to the client options that contains the http pipeline policies.
     */
    void ConfigureClientOptions(Azure::Core::_internal::ClientOptions& clientOptions);

    /**
     * @brief Create and return client options with test configuration set in the environment.
     *
     * @note If test proxy env var is set, the proxy policy is added to the \p clientOptions.
     */
    template <class T> T InitClientOptions()
    {
      T options;
      ConfigureClientOptions(options);
      return options;
    }

    /**
     * @brief Returns the test proxy.
     */
    std::string GetTestProxy() const { return m_proxy; }
  };
}} // namespace Azure::Perf

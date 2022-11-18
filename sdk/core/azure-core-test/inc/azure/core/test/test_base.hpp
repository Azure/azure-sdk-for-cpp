// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * Base class for running live and playback tests using the interceptor manager
 */

#include <gtest/gtest.h>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/test/test_proxy_manager.hpp>
#include <azure/identity/client_secret_credential.hpp>

#include <chrono>
#include <memory>
#include <regex>
#include <thread>

#define CHECK_SKIP_TEST() \
  std::string const readTestNameAndUpdateTestContext = GetTestName(); \
  if (shouldSkipTest()) \
  { \
    GTEST_SKIP(); \
  }

using namespace std::chrono_literals;

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief The base class provides the tools for a test to use the Record&PlayBack functionalities.
   *
   */
  class TestBase : public ::testing::Test {

  private:
    /**
     * @brief Whenever a test case is skipped
     *
     */
    bool m_wasSkipped = false;
    void PrepareOptions(Azure::Core::_internal::ClientOptions& options)
    {
      // Set up client options depending on the test-mode
      if (m_testContext.IsPlaybackMode())
      {        
        // Playback mode uses:
        //  - playback transport adapter to read and return payload from json files
        m_testProxy->SetStartPlaybackMode();

        options.PerRetryPolicies.push_back(m_testProxy->GetTestProxyPolicy());
      }
      else if (!m_testContext.IsLiveMode())
      {
        // Record mode uses:
        //  - curl or winhttp transport adapter
        //  - Recording policy. Intercept server responses to create json files
        // AZURE_TEST_RECORDING_DIR is exported by CMAKE
        m_testProxy->SetStartRecordMode();
        options.PerRetryPolicies.push_back(m_testProxy->GetTestProxyPolicy());
      }
    }

    void PrepareClientCredential(std::shared_ptr<Core::Credentials::TokenCredential>& credential)
    {
      if (m_testContext.IsPlaybackMode())
      {
        // Playback mode uses:
        //  - never-expiring test credential to never require a token
        credential = m_testProxy->GetTestCredential();
      }
    }

    // Call this method to update client options with the required configuration to
    // support Record & Playback.
    // If Playback or Record is not set, no changes will be done to the clientOptions or credential.
    // Call this before creating the SDK client
    void PrepareClientOptions(
        std::shared_ptr<Core::Credentials::TokenCredential>& credential,
        Azure::Core::_internal::ClientOptions& options)
    {
      // Set up client options depending on the test-mode
      PrepareOptions(options);
      PrepareClientCredential(credential);
    }

    std::string Sanitize(std::string const& src)
    {
      std::string updated(src);
      std::replace(updated.begin(), updated.end(), '/', '-');
      return RemovePreffix(updated);
    }

    void SkipTest()
    {
      m_wasSkipped = true;
      GTEST_SKIP();
    }

    std::string RemovePreffix(std::string const& src)
    {
      std::string updated(src);
      // Remove special marker for LIVEONLY
      auto const noPrefix
          = std::regex_replace(updated, std::regex(TestContextManager::LiveOnlyToken), "");
      if (noPrefix != updated)
      {
        if (m_testContext.TestMode == TestMode::RECORD)
        {
          TestLog("Test is expected to run on LIVE mode only. Recording won't be created.");
        }
        else if (m_testContext.TestMode == TestMode::PLAYBACK)
        {
          TestLog("Test is expected to run on LIVE mode only. Skipping test on playback mode.");
          SkipTest();
        }
        m_testContext.LiveOnly = true;
        return noPrefix;
      }
      return updated;
    }

  protected:
    Azure::Core::Test::TestContextManager m_testContext;
    std::unique_ptr<Azure::Core::Test::TestProxyManager> m_testProxy;

    bool shouldSkipTest() { return m_wasSkipped; }

    inline void ValidateSkippingTest()
    {
      if (m_wasSkipped)
      {
        GTEST_SKIP();
      }
    }

    bool IsValidTime(const Azure::DateTime& datetime)
    {
      // Playback won't check dates
      if (m_testContext.IsPlaybackMode())
      {
        return true;
      }

      // We assume datetime within a week is valid.
      const auto minTime = std::chrono::system_clock::now() - std::chrono::hours(24 * 7);
      const auto maxTime = std::chrono::system_clock::now() + std::chrono::hours(24 * 7);
      return datetime > minTime && datetime < maxTime;
    }

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating
    std::string GetTestName(bool sanitize = true)
    {
      std::string testName(::testing::UnitTest::GetInstance()->current_test_info()->name());
      if (sanitize)
      {
        // replace `/` for `-`. Parameterized tests adds this char automatically to join the test
        // name and the parameter suffix.
        testName = Sanitize(testName);
      }

      return RemovePreffix(testName);
    }

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating
    std::string GetTestNameLowerCase(bool sanitize = true)
    {
      std::string testName(GetTestName(sanitize));
      return Azure::Core::_internal::StringExtensions::ToLower(testName);
    }

    /**
     * @brief Get test name with suffix if ENV variable is set.
     *
     * @param sanitize Sanitize the input and remove special characters. Default true.
     * @param suffixEnvName Env variable containing the suffix. Default AZURE_LIVE_TEST_SUFFIX.
     *
     * @returns Test name.
     */
    std::string GetTestNameSuffix(
        bool sanitize = true,
        std::string suffixEnvName = "AZURE_LIVE_TEST_SUFFIX")
    {
      std::string baseValue = Azure::Core::Test::TestBase::GetTestName(sanitize);

      std::string suffix = Azure::Core::_internal::Environment::GetVariable(suffixEnvName.c_str());

      if (suffix.length() > 0)
      {
        baseValue = "-" + suffix;
      }

      return baseValue;
    }

    // Creates the sdk client for testing.
    // The client will be set for record and playback before it is created.
    Azure::Core::Credentials::TokenCredentialOptions GetTokenCredentialOptions()
    {
      // Run instrumentation before creating the client
      Azure::Core::Credentials::TokenCredentialOptions options;
      PrepareOptions(options);
      return options;
    }

    // Creates the sdk client for testing.
    // The client will be set for record and playback before it is created.
    template <class T, class O>
    std::unique_ptr<T> InitTestClient(
        std::string const& url,
        std::shared_ptr<Core::Credentials::TokenCredential>& credential,
        O& options)
    {
      // Run instrumentation before creating the client
      PrepareClientOptions(credential, options);
      return std::make_unique<T>(url, credential, options);
    }

    template <class T> T InitClientOptions()
    {
      // Run instrumentation before creating the client
      T options;
      PrepareOptions(options);
      return options;
    }

    std::shared_ptr<Azure::Core::Credentials::TokenCredential> CreateClientSecretCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientSecret)
    {
      if (m_testContext.IsPlaybackMode())
      {
        // Playback mode uses:
        //  - never-expiring test credential to never require a token
        return m_testProxy->GetTestCredential();
      }
      else
      {
        return std::make_shared<Azure::Identity::ClientSecretCredential>(
            tenantId, clientId, clientSecret);
      }
    }

    // Updates the time when test is on playback
    void UpdateWaitingTime(std::chrono::milliseconds& current)
    {
      if (m_testContext.IsPlaybackMode())
      {
        current = 0ms;
      }
    }

    std::chrono::seconds PollInterval(std::chrono::seconds const& seconds = 1s)
    {
      if (m_testContext.IsPlaybackMode())
      {
        return 0s;
      }
      return seconds;
    }

    // Util for tests to introduce delays
    void TestSleep(std::chrono::milliseconds const& ms = 1s)
    {
      if (m_testContext.IsPlaybackMode())
      {
        return;
      }
      std::this_thread::sleep_for(ms);
    }

    void TestLog(std::string const& message)
    {
      using Azure::Core::Diagnostics::Logger;
      using Azure::Core::Diagnostics::_internal::Log;
      Log::Write(
          Logger::Level::Verbose,
          "Test Log from: [ " + m_testContext.GetTestPlaybackRecordingName() + " ] - " + message);
    }

    /**
     * @brief Utility function used by tests to retrieve env vars
     *
     * @param name Environment variable name to retrieve.
     *
     * @return The value of the environment variable retrieved.
     *
     * @note If AZURE_TENANT_ID, AZURE_CLIENT_ID, or AZURE_CLIENT_SECRET are not available in the
     * environment, the AZURE_SERVICE_DIRECTORY environment variable is used to set those values
     * with the values emitted by the New-TestResources.ps1 script.
     *
     * @note The Azure CI pipeline upper cases all environment variables defined in the pipeline.
     * Since some operating systems have case sensitive environment variables, on debug builds, this
     * function ensures that the environment variable being retrieved is all upper case.
     *
     */
    std::string GetEnv(std::string const& name)
    {
#if !defined(NDEBUG)
      // The azure CI pipeline uppercases all EnvVar values from ci.yml files.
      // That means that any mixed case strings will not be found when run from the CI
      // pipeline. Check to make sure that the developer only passed in an upper case environment
      // variable.
      {
        if (name != Azure::Core::_internal::StringExtensions::ToUpper(name))
        {
          throw std::runtime_error("All Azure SDK environment variables must be all upper case.");
        }
      }
#endif
      auto ret = Azure::Core::_internal::Environment::GetVariable(name.c_str());
      if (ret.empty())
      {
        static const char azurePrefix[] = "AZURE_";
        if (!m_testContext.IsPlaybackMode() && name.find(azurePrefix) == 0)
        {
          std::string serviceDirectory
              = Azure::Core::_internal::Environment::GetVariable("AZURE_SERVICE_DIRECTORY");
          if (serviceDirectory.empty())
          {
            throw std::runtime_error(
                "Could not find a value for " + name
                + " and AZURE_SERVICE_DIRECTORY was not defined. Define either " + name
                + " or AZURE_SERVICE_DIRECTORY to resolve.");
          }
          // Upper case the serviceName environment variable because all ci.yml environment
          // variables are upper cased.
          std::string serviceDirectoryEnvVar
              = Azure::Core::_internal::StringExtensions::ToUpper(serviceDirectory);
          serviceDirectoryEnvVar += name.substr(sizeof(azurePrefix) - 2);
          ret = Azure::Core::_internal::Environment::GetVariable(serviceDirectoryEnvVar.c_str());
          if (!ret.empty())
          {
            return ret;
          }
        }
        throw std::runtime_error("Missing required environment variable: " + name);
      }

      return ret;
    }

    // Util to set recording path

    /**
     * @brief Run before each test.
     *
     * @param baseRecordingPath - the base recording path to be used for this test. Normally this is
     * `AZURE_TEST_RECORDING_DIR`.
     *
     * For example:
     *
     * \code{.cpp}
     *  Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
     * \endcode
     *
     */
    void SetUpTestBase(std::string const& baseRecordingPath)
    {
      // Init interceptor from PlayBackRecorder
      std::string recordingPath(baseRecordingPath);
      recordingPath.append("/recordings");

      m_testContext.TestMode = Azure::Core::Test::TestProxyManager::GetTestMode();
      // Use the test info to init the test context and interceptor.
      auto testNameInfo = ::testing::UnitTest::GetInstance()->current_test_info();
      // set the interceptor for the current test
      m_testContext.RenameTest(
          Sanitize(testNameInfo->test_suite_name()), Sanitize(testNameInfo->name()));
      m_testContext.RecordingPath = recordingPath;
      m_testProxy = std::make_unique<Azure::Core::Test::TestProxyManager>(m_testContext);
    }

    /**
     * @brief Run after each test
     *
     * @note: If a test case overrides the TearDown method, it MUST call the `TestBase::TearDown`
     * method, or test recordings will fail to be generated.
     *
     */
    void TearDown() override;

  public:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite()
    {
      std::system("pwsh -NoProfile -ExecutionPolicy Unrestricted testproxy.ps1");
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite()
    {
      std::system("pwsh -NoProfile -ExecutionPolicy Unrestricted stopProxy.ps1");
    }
  };
}}} // namespace Azure::Core::Test

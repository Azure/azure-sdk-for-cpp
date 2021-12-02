// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * Base class for running live and playback tests using the interceptor manager
 */

#include <gtest/gtest.h>

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"

#include <chrono>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief The base class provides the tools for a test to use the Record&PlayBack functionalities.
   *
   */
  class TestBase : public ::testing::Test {

  private:
    // Call this method to update client options with the required configuration to
    // support Record & Playback.
    // If Playback or Record is not set, no changes will be done to the clientOptions or credential.
    // Call this before creating the SDK client
    void PrepareClientOptions(
        std::shared_ptr<Core::Credentials::TokenCredential>** credential,
        Azure::Core::_internal::ClientOptions& options)
    {
      // Set up client options depending on the test-mode
      if (m_testContext.IsPlaybackMode())
      {
        // Playback mode uses:
        //  - playback transport adapter to read and return payload from json files
        //  - never-expiring test credential to never require a token
        options.Transport.Transport = m_interceptor->GetPlaybackTransport();
        **credential = m_interceptor->GetTestCredential();
      }
      else if (!m_testContext.IsLiveMode())
      {
        // Record mode uses:
        //  - curl or winhttp transport adapter
        //  - Recording policy. Intercept server responses to create json files
        // AZURE_TEST_RECORDING_DIR is exported by CMAKE
        options.PerRetryPolicies.push_back(m_interceptor->GetRecordPolicy());
      }
    }

    std::string Sanitize(std::string const& src)
    {
      std::string updated(src);
      std::replace(updated.begin(), updated.end(), '/', '-');
      return updated;
    }

  protected:
    Azure::Core::Test::TestContextManager m_testContext;
    std::unique_ptr<Azure::Core::Test::InterceptorManager> m_interceptor;

    // Reads the current test instance name.
    // Name gets also sanitized (special chars are removed) to avoid issues when recording or
    // creating
    std::string GetTestName(bool sanitize = false)
    {
      std::string testName(::testing::UnitTest::GetInstance()->current_test_info()->name());
      if (sanitize)
      {
        // replace `/` for `-`. Parameterized tests adds this char automatically to join the test
        // name and the parameter suffix.
        return Sanitize(testName);
      }

      return testName;
    }

    // Creates the sdk client for testing.
    // The client will be set for record and playback before it is created.
    template <class T, class O>
    std::unique_ptr<T> InitTestClient(
        std::string const& url,
        std::shared_ptr<Core::Credentials::TokenCredential>* credential,
        O& options)
    {
      // Run instrumentation before creating the client
      PrepareClientOptions(&credential, options);
      return std::make_unique<T>(url, *credential, options);
    }

    // Updates the time when test is on playback
    void UpdateWaitingTime(std::chrono::milliseconds& current)
    {
      if (m_testContext.IsPlaybackMode())
      {
        current = 0ms;
      }
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

    // Util for tests getting env vars
    std::string GetEnv(const std::string& name)
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
      const char* ret = std::getenv(name.data());
#pragma warning(pop)
#else
      const char* ret = std::getenv(name.data());
#endif

      if (!ret)
      {
        throw std::runtime_error("Missing required environment variable: " + name);
      }

      return std::string(ret);
    }

    // Util to set recording path

    /**
     * @brief Run before each test.
     *
     */
    void SetUpTestBase(std::string const& baseRecordingPath)
    {
      // Init interceptor from PlayBackRecorder
      std::string recordingPath(baseRecordingPath);
      recordingPath.append("/recordings");

      // Use the test info to init the test context and interceptor.
      auto testNameInfo = ::testing::UnitTest::GetInstance()->current_test_info();

      // set the interceptor for the current test
      m_testContext.RenameTest(
          Sanitize(testNameInfo->test_suite_name()), Sanitize(testNameInfo->name()));
      m_testContext.RecordingPath = recordingPath;
      m_testContext.TestMode = Azure::Core::Test::InterceptorManager::GetTestMode();
      m_interceptor = std::make_unique<Azure::Core::Test::InterceptorManager>(m_testContext);
    }

    /**
     * @brief Run after each test
     *
     */
    void TearDown() override;
  };
}}} // namespace Azure::Core::Test

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * This class handles managing context about a test.
 */

#pragma once

#include <gtest/gtest.h>
#include <string>

#include "azure/core/test/network_models.hpp"

namespace Azure { namespace Core { namespace Test {
  class TestContextManager {
  private:
    std::string m_testName;
    std::string m_testSuite;
    Azure::Core::Test::TestMode m_testMode;
    bool m_liveOnly;
    bool m_testRan;

  protected:
    constexpr static const char* LiveOnlyToken = "LIVE";

  public:
    /**
     * @brief Construct a new Test Context Manager object based on the test info.
     *
     * @param testName The test that is running.
     * @param testMode The test mode for the context.
     */
    TestContextManager(::testing::TestInfo const* testInfo, Azure::Core::Test::TestMode testMode)
        : m_testName(std::string(testInfo->test_case_name())),
          m_testSuite(std::string(testInfo->test_suite_name())), m_testMode(testMode)
    {
      // Naming a test with a prefix `LIVE` will set it up to be only live mode supported.
      // It won't be recorded and it won't be ran when playback mode is on.
      std::string liveOnlyToken(Azure::Core::Test::TestContextManager::LiveOnlyToken);
      if (m_testName.size() > liveOnlyToken.size())
      {
        if (m_testName.find(liveOnlyToken) == 0)
        {
          m_liveOnly = true;
        }
      }
    }

    /**
     * @brief Get the Test Name.
     *
     * @return The test name.
     */
    std::string const& GetTestName() const { return m_testName; }

    /**
     * @brief The test suite name plus the test name.
     *
     * @return testSuiteName.testName
     */
    std::string GetTestPlaybackRecordingName() const
    {
      std::string fullName(m_testSuite);
      fullName.append(".");
      fullName.append(m_testName);

      return fullName;
    }

    /**
     * @brief Get the Test Mode object for the current test.
     *
     * @return Either LIVE, PLAYBACK or RECORD.
     */
    Azure::Core::Test::TestMode GetTestMode() const { return m_testMode; }

    /**
     * @brief Check if test is expected to be recorded.
     *
     * @return `true` when test is expected to be recorded or `false` when it is not.
     */
    bool DoNotRecordTest() const
    {
      return m_testMode != Azure::Core::Test::TestMode::RECORD || m_liveOnly;
    }
  };

}}} // namespace Azure::Core::Test

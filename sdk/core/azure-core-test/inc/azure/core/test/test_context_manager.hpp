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
  public:
    std::string RecordingPath;
    Azure::Core::Test::TestMode TestMode;
    bool LiveOnly = false;

    TestContextManager() { SetLiveOnly(); }

    void RenameTest(std::string const& testName)
    {
      m_testName = testName;
      SetLiveOnly();
    }

    void RenameTest(std::string const& testSuite, std::string const& testName)
    {
      m_testSuite = testSuite;
      RenameTest(testName);
    }

    /**
     * @brief The test suite name plus the test name.
     *
     * @return m_testSuiteName.m_testName
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
    Azure::Core::Test::TestMode GetTestMode() const { return TestMode; }

    /**
     * @brief Check if test is expected to be recorded.
     *
     * @return `true` when test is expected to be recorded or `false` when it is not.
     */
    bool DoNotRecordTest() const
    {
      return TestMode != Azure::Core::Test::TestMode::RECORD || LiveOnly;
    }

    bool IsPlaybackMode() const { return TestMode == Azure::Core::Test::TestMode::PLAYBACK; }
    bool IsLiveMode() const { return TestMode == Azure::Core::Test::TestMode::LIVE; }

  protected:
    constexpr static const char* LiveOnlyToken = "LIVE";

  private:
    std::string m_testName;
    std::string m_testSuite;
    void SetLiveOnly()
    {
      // Naming a test with a prefix `LIVE` will set it up to be only live mode supported.
      // It won't be recorded and it won't be ran when playback mode is on.
      std::string liveOnlyToken(Azure::Core::Test::TestContextManager::LiveOnlyToken);
      if (m_testName.size() > liveOnlyToken.size())
      {
        if (m_testName.find(liveOnlyToken) == 0)
        {
          LiveOnly = true;
        }
      }
    }
  };

}}} // namespace Azure::Core::Test

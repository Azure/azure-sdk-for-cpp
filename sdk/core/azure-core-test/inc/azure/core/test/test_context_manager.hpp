// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * This class handles managing context about a test.
 */

#pragma once

#include "azure/core/test/network_models.hpp"

#include <string>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Test {
  /**
   * @brief The test context is used to hold information about a test, such as the recording path,
   * running test mode, name, etc.
   *
   */
  class TestContextManager {
  public:
    /**
     * @brief The path where the asset.json for the current test exists, will be passed as part of
     * the playback request to the test-proxy via the manager.
     *
     */
    std::string AssetsPath;
    /**
     * @brief The path where the tests recordings are written.
     *
     */
    std::string RecordingPath;

    /**
     * @brief The mode how the test is running.
     *
     */
    Azure::Core::Test::TestMode TestMode;

    /**
     * @brief Whenever the test must be ran on live mode only.
     *
     * @remark This configuration allow tests to ignore the recording or playback setting and run
     * without it and amount other tests which are using the recording and playback.
     *
     */
    bool LiveOnly = false;

    /**
     * @brief Whenever the test must not be ran on live mode.
     *
     * @remark This configuration allow tests to ignore live mode
     *
     */
    bool PlaybackOnly = false;

    /**
     * @brief Construct a new Test Context Manager object
     *
     */
    TestContextManager()
    {
      auto testNameInfo = ::testing::UnitTest::GetInstance()->current_test_info();
      SetRunFlags(testNameInfo->name());
    }

    /**
     * @brief Change the name of the running test.
     *
     * @param testName The new name for the test.
     */
    void RenameTest(std::string const& testName)
    {
      m_testName = testName;
    }

    /**
     * @brief Change the name of the test suite and test name.
     *
     * @param testSuite The new name for the test suite.
     * @param testName The new name for the test name.
     */
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

    std::string GetTestRecordingPathName() const
    {
      std::string fullName(RecordingPath);
      fullName.append("/");
      fullName.append(GetTestPlaybackRecordingName());
      fullName.append(".json");

      return fullName;
    }

    std::string GetTestName() const { return m_testName; }
    std::string GetTestSuiteName() const { return m_testSuite; }

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

    /**
     * @brief Whenever the test is running on playback mode.
     *
     * @return true if the test is using recorded data as server responses.
     */
    bool IsPlaybackMode() const { return TestMode == Azure::Core::Test::TestMode::PLAYBACK; }

    /**
     * @brief Whenever the test is running on live mode.
     *
     * @return true if the test is not recording or returning recorded data.
     */
    bool IsLiveMode() const { return TestMode == Azure::Core::Test::TestMode::LIVE; }

    constexpr static const char* LiveOnlyToken = "_LIVEONLY_";
    constexpr static const char* PlaybackOnlyToken = "_RECORDEDONLY_";

    std::string RecordingId;

  private:
    std::string m_testName;
    std::string m_testSuite;
    void SetRunFlags(std::string const& testName)
    {
      // Naming a test with a prefix `LIVE` will set it up to be only live mode supported.
      // It won't be recorded and it won't be ran when playback mode is on.
      std::string liveOnlyToken{Azure::Core::Test::TestContextManager::LiveOnlyToken};
      std::string playbackOnlyToken{Azure::Core::Test::TestContextManager::PlaybackOnlyToken};
      LiveOnly = false;
      PlaybackOnly = false;
      if (testName.size() > liveOnlyToken.size())
      {
        if (testName.find(liveOnlyToken) != std::string::npos)
        {
          LiveOnly = true;
        }
      }
      // Naming a test with a suffix `PLAYBACKONLY` will set it up to be only PAYBACK/RECORD mode
      // supported. it won't be running in live mode.
      if (testName.size() > playbackOnlyToken.size())
      {
        if (testName.find(playbackOnlyToken) != std::string::npos)
        {
          PlaybackOnly = true;
        }
      }
    }
  };

}}} // namespace Azure::Core::Test

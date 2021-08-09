// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * Base class for running live and playback tests using the interceptor manager
 */

#include <gtest/gtest.h>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"

#include <memory>

namespace Azure { namespace Core { namespace Test {

  class TestBase : public ::testing::Test {

  protected:
    Azure::Core::Test::TestContextManager m_testContext;
    std::unique_ptr<Azure::Core::Test::InterceptorManager> m_interceptor;

    /**
     * @brief Run before each test
     *
     */
    void SetUpBase(std::string const& recordingPath)
    {
      // Use the test info to init the test context and interceptor.
      auto testNameInfo = ::testing::UnitTest::GetInstance()->current_test_info();

      // set the interceptor for the current test
      m_testContext.RenameTest(testNameInfo->test_suite_name(), testNameInfo->name());
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

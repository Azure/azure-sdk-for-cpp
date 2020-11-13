// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * Base class for running live and playback tests using the interceptor manager
 */

#include <gtest/gtest.h>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"

namespace Azure { namespace Core { namespace Test {

  class TestBase : public ::testing::Test {
  private:
    static Azure::Core::Test::TestMode testMode;

  public:
    Azure::Core::Test::InterceptorManager m_interceptor = Azure::Core::Test::InterceptorManager();

    /**
     * @brief Set the test mode
     *
     */
    static void SetUpTestSuite()
    {
      testMode = Azure::Core::Test::InterceptorManager::GetTestMode();
    }

    /**
     * @brief Run before each test
     *
     */
    void SetUp() override
    {
      // Create test content from gtest Test Info
      auto testNameInfo = ::testing::UnitTest::GetInstance()->current_test_info();
      Azure::Core::Test::TestContextManager testContext(testNameInfo, testMode);

      // set the interceptor for the current test
      m_interceptor = Azure::Core::Test::InterceptorManager(testContext);
    }
  };
}}} // namespace Azure::Core::Test

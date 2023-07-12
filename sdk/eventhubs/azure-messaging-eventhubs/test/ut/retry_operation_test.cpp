// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
#include "private/retry_operation.hpp"

#include <functional>

#include <gtest/gtest.h>

namespace LocalTest {
bool testFunc() { return true; }
bool testNegative() { return false; }
Azure::Core::Http::Policies::RetryOptions retryOptions;
} // namespace LocalTest

namespace Azure { namespace Messaging { namespace EventHubs { namespace _internal { namespace Test {
  class RetryOperationTest : public EventHubsTestBase {
  };
  TEST_F(RetryOperationTest, ExecuteTrue)
  {
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_TRUE(retryOp.Execute(LocalTest::testFunc));
  }

  TEST_F(RetryOperationTest, ExecuteFalse)
  {
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_FALSE(retryOp.Execute(LocalTest::testNegative));
  }

  TEST_F(RetryOperationTest, ShouldRetryTrue1)
  {
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_FALSE(retryOp.ShouldRetry(true, 0, retryAfter));
  }

  TEST_F(RetryOperationTest, ShouldRetryTrue2)
  {
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_FALSE(retryOp.ShouldRetry(true, LocalTest::retryOptions.MaxRetries, retryAfter));
  }

  TEST_F(RetryOperationTest, ShouldRetryFalse1)
  {
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_TRUE(retryOp.ShouldRetry(false, 0, retryAfter));
  }

  TEST_F(RetryOperationTest, ShouldRetryFalse2)
  {
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(LocalTest::retryOptions);
    EXPECT_FALSE(retryOp.ShouldRetry(false, LocalTest::retryOptions.MaxRetries, retryAfter));
  }
}}}}} // namespace Azure::Messaging::EventHubs::_internal::Test

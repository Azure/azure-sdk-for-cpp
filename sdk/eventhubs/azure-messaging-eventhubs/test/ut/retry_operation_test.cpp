// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_test_base.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <functional>

#include <gtest/gtest.h>

namespace LocalTest {
bool testFunc() { return true; }
bool testNegative() { return false; }
Azure::Core::Http::Policies::RetryOptions retryOptions;

// Fast retry options keep regression tests for issue #7130 quick; the production
// defaults (3 retries, 800ms base delay) would add several seconds of backoff per test.
Azure::Core::Http::Policies::RetryOptions MakeFastRetryOptions(int32_t maxRetries = 3)
{
  Azure::Core::Http::Policies::RetryOptions opts;
  opts.MaxRetries = maxRetries;
  opts.RetryDelay = std::chrono::milliseconds(1);
  opts.MaxRetryDelay = std::chrono::milliseconds(2);
  return opts;
}
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

  // Regression tests for issue #7130: RetryOperation::Execute must rethrow the last
  // exception when all retry attempts have been exhausted; previously it returned false
  // and silently dropped the failure.
  TEST_F(RetryOperationTest, RethrowsLastEventHubsExceptionWhenRetriesExhausted)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    int callCount = 0;
    auto alwaysThrows = [&callCount]() -> bool {
      ++callCount;
      throw Azure::Messaging::EventHubs::EventHubsException(
          "transient failure attempt " + std::to_string(callCount));
    };

    try
    {
      retryOp.Execute(alwaysThrows);
      FAIL() << "Expected EventHubsException to be rethrown after retries were exhausted.";
    }
    catch (Azure::Messaging::EventHubs::EventHubsException const& e)
    {
      EXPECT_STREQ("transient failure attempt 3", e.what());
    }
    EXPECT_EQ(3, callCount);
  }

  TEST_F(RetryOperationTest, RethrowsLastStdExceptionWhenRetriesExhausted)
  {
    auto opts = LocalTest::MakeFastRetryOptions(2);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    int callCount = 0;
    auto alwaysThrows = [&callCount]() -> bool {
      ++callCount;
      throw std::runtime_error("network blip " + std::to_string(callCount));
    };

    try
    {
      retryOp.Execute(alwaysThrows);
      FAIL() << "Expected std::runtime_error to be rethrown after retries were exhausted.";
    }
    catch (std::runtime_error const& e)
    {
      EXPECT_STREQ("network blip 2", e.what());
    }
    EXPECT_EQ(2, callCount);
  }

  TEST_F(RetryOperationTest, ThrowsImmediatelyOnFatalEventHubsException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    int callCount = 0;
    auto throwsFatal = [&callCount]() -> bool {
      ++callCount;
      Azure::Messaging::EventHubs::EventHubsException ex("message too big");
      ex.ErrorCondition = "amqp:link:message-size-exceeded";
      throw ex;
    };

    EXPECT_THROW(retryOp.Execute(throwsFatal), Azure::Messaging::EventHubs::EventHubsException);
    EXPECT_EQ(1, callCount) << "Fatal exception must not be retried.";
  }

  TEST_F(RetryOperationTest, SucceedsAfterTransientException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    int callCount = 0;
    auto eventuallySucceeds = [&callCount]() -> bool {
      ++callCount;
      if (callCount == 1)
      {
        throw Azure::Messaging::EventHubs::EventHubsException("first attempt fails");
      }
      return true;
    };

    EXPECT_TRUE(retryOp.Execute(eventuallySucceeds));
    EXPECT_EQ(2, callCount);
  }

  TEST_F(RetryOperationTest, FalseAfterTransientExceptionDoesNotRethrow)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    int callCount = 0;
    auto throwsThenReturnsFalse = [&callCount]() -> bool {
      ++callCount;
      if (callCount == 1)
      {
        throw Azure::Messaging::EventHubs::EventHubsException("first attempt fails");
      }
      return false;
    };

    // Second attempt returns false cleanly. ShouldRetry(false=response, retryCount=1)
    // returns true, so the loop retries; on attempt 3 the loop terminates and Execute
    // returns false. The exception from attempt 1 must not be rethrown.
    EXPECT_NO_THROW({ EXPECT_FALSE(retryOp.Execute(throwsThenReturnsFalse)); });
    EXPECT_EQ(opts.MaxRetries, callCount);
  }
}}}}} // namespace Azure::Messaging::EventHubs::_internal::Test

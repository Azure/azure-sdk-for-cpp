// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_test_base.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace LocalTest {
Azure::Core::Http::Policies::RetryOptions MakeFastRetryOptions(int32_t maxRetries = 3)
{
  Azure::Core::Http::Policies::RetryOptions opts;
  opts.MaxRetries = maxRetries;
  opts.RetryDelay = std::chrono::milliseconds(1);
  opts.MaxRetryDelay = std::chrono::milliseconds(2);
  return opts;
}

Azure::Messaging::EventHubs::EventHubsException MakeEventHubsException(
    Azure::Core::Amqp::Models::_internal::AmqpErrorCondition const& condition,
    std::string const& message)
{
  Azure::Core::Amqp::Models::_internal::AmqpError error;
  error.Condition = condition;
  error.Description = message;
  return Azure::Messaging::EventHubs::_detail::EventHubsExceptionFactory::CreateEventHubsException(
      error);
}

// Collects the log lines that the code under test writes. The destructor
// removes the listener before the vector goes away, because a background thread
// can write to the log while this object is destroyed.
class LogCapture final {
public:
  LogCapture()
  {
    Azure::Core::Diagnostics::Logger::SetListener(
        [this](Azure::Core::Diagnostics::Logger::Level level, std::string const& message) {
          std::lock_guard<std::mutex> guard(m_mutex);
          m_lines.emplace_back(level, message);
        });
    Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Verbose);
  }

  ~LogCapture() { Azure::Core::Diagnostics::Logger::SetListener(nullptr); }

  LogCapture(LogCapture const&) = delete;
  LogCapture& operator=(LogCapture const&) = delete;

  std::vector<std::string> Lines(Azure::Core::Diagnostics::Logger::Level level)
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    std::vector<std::string> result;
    for (auto const& line : m_lines)
    {
      if (line.first == level)
      {
        result.push_back(line.second);
      }
    }
    return result;
  }

  void Clear()
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_lines.clear();
  }

private:
  std::mutex m_mutex;
  std::vector<std::pair<Azure::Core::Diagnostics::Logger::Level, std::string>> m_lines;
};

std::vector<std::string> LinesContaining(
    std::vector<std::string> const& lines,
    std::string const& fragment)
{
  std::vector<std::string> result;
  for (auto const& line : lines)
  {
    if (line.find(fragment) != std::string::npos)
    {
      result.push_back(line);
    }
  }
  return result;
}
} // namespace LocalTest

namespace Azure { namespace Messaging { namespace EventHubs { namespace _internal { namespace Test {
  class RetryOperationTest : public EventHubsTestBase {
  };

  TEST_F(RetryOperationTest, ExecuteTrue)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;

    EXPECT_TRUE(retryOp.Execute([]() { return true; }, context));
  }

  TEST_F(RetryOperationTest, ExecuteFalse)
  {
    auto opts = LocalTest::MakeFastRetryOptions(2);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    EXPECT_FALSE(retryOp.Execute(
        [&callCount]() {
          ++callCount;
          return false;
        },
        context));
    EXPECT_EQ(opts.MaxRetries + 1, callCount);
  }

  TEST_F(RetryOperationTest, ShouldRetryTrue1)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    EXPECT_FALSE(retryOp.ShouldRetry(true, 0, retryAfter));
  }

  TEST_F(RetryOperationTest, ShouldRetryTrue2)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    EXPECT_FALSE(retryOp.ShouldRetry(true, opts.MaxRetries, retryAfter));
  }

  TEST_F(RetryOperationTest, ShouldRetryFalse1)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    EXPECT_TRUE(retryOp.ShouldRetry(false, 0, retryAfter, 1.0));
    EXPECT_EQ(opts.RetryDelay, retryAfter);

    EXPECT_TRUE(retryOp.ShouldRetry(false, 1, retryAfter, 1.0));
    EXPECT_EQ(opts.RetryDelay * 2, retryAfter);
  }

  TEST_F(RetryOperationTest, ShouldRetryFalse2)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    std::chrono::milliseconds retryAfter{};
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);

    EXPECT_FALSE(retryOp.ShouldRetry(false, opts.MaxRetries, retryAfter));
    EXPECT_EQ(std::chrono::milliseconds::zero(), retryAfter);
  }

  TEST_F(RetryOperationTest, RethrowsLastEventHubsExceptionWhenRetriesExhausted)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto alwaysThrows = [&callCount]() -> bool {
      ++callCount;
      throw LocalTest::MakeEventHubsException(
          Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
          "transient failure attempt " + std::to_string(callCount));
    };

    try
    {
      retryOp.Execute(alwaysThrows, context);
      FAIL() << "Expected EventHubsException to be rethrown after retries were exhausted.";
    }
    catch (Azure::Messaging::EventHubs::EventHubsException const& e)
    {
      EXPECT_STREQ("transient failure attempt 4", e.what());
    }
    EXPECT_EQ(opts.MaxRetries + 1, callCount);
  }

  TEST_F(RetryOperationTest, RethrowsLastRuntimeErrorWhenRetriesExhausted)
  {
    auto opts = LocalTest::MakeFastRetryOptions(2);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto alwaysThrows = [&callCount]() -> bool {
      ++callCount;
      throw std::runtime_error("Could not send message attempt " + std::to_string(callCount));
    };

    try
    {
      retryOp.Execute(alwaysThrows, context);
      FAIL() << "Expected std::runtime_error to be rethrown after retries were exhausted.";
    }
    catch (std::runtime_error const& e)
    {
      EXPECT_STREQ("Could not send message attempt 3", e.what());
    }
    EXPECT_EQ(opts.MaxRetries + 1, callCount);
  }

  TEST_F(RetryOperationTest, RetriesEmptyEventHubsCondition)
  {
    auto opts = LocalTest::MakeFastRetryOptions(2);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto succeedsAfterEmptyError = [&callCount]() -> bool {
      ++callCount;
      if (callCount == 1)
      {
        auto exception = LocalTest::MakeEventHubsException(
            Azure::Core::Amqp::Models::_internal::AmqpErrorCondition{},
            "unknown communication failure");
        EXPECT_TRUE(exception.IsTransient);
        throw exception;
      }
      return true;
    };

    EXPECT_TRUE(retryOp.Execute(succeedsAfterEmptyError, context));
    EXPECT_EQ(2, callCount);
  }

  TEST_F(RetryOperationTest, RetriesAllowlistedEventHubsConditions)
  {
    auto opts = LocalTest::MakeFastRetryOptions(2);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    Azure::Core::Amqp::Models::_internal::AmqpErrorCondition errorConditions[]
        = {Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ServerBusyError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::InternalError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::LinkDetachForced,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ConnectionForced,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ConnectionFramingError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ProtonIo,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::NotFound,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::IllegalState};

    for (auto const& errorCondition : errorConditions)
    {
      int callCount = 0;
      auto succeedsAfterTransientError = [&callCount, errorCondition]() -> bool {
        ++callCount;
        if (callCount == 1)
        {
          auto exception = LocalTest::MakeEventHubsException(errorCondition, "transient failure");
          EXPECT_TRUE(exception.IsTransient);
          throw exception;
        }
        return true;
      };

      EXPECT_TRUE(retryOp.Execute(succeedsAfterTransientError, context));
      EXPECT_EQ(2, callCount) << errorCondition.ToString();
    }
  }

  TEST_F(RetryOperationTest, DoesNotRetryUnknownOrKnownNonTransientEventHubsConditions)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    Azure::Core::Amqp::Models::_internal::AmqpErrorCondition errorConditions[]
        = {Azure::Core::Amqp::Models::_internal::AmqpErrorCondition{
               "com.microsoft:future-unknown-error"},
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::UnauthorizedAccess,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::DecodeError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ResourceLimitExceeded,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ResourceLocked,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::NotAllowed,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::InvalidField,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::NotImplemented,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::PreconditionFailed,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ResourceDeleted,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::FrameSizeTooSmall,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::LinkStolen,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::LinkPayloadSizeExceeded,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ArgumentError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ArgumentOutOfRangeError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::EntityDisabledError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::PartitionNotOwnedError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::StoreLockLostError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::PublisherRevokedError,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TrackingIdProperty,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::OperationCancelled,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::MessageLockLost,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::SessionLockLost,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::SessionCannotBeLocked,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::MessageNotFound,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::SessionNotFound,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::EntityAlreadyExists,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::ConnectionRedirect,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::LinkRedirect,
           Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TransferLimitExceeded};

    for (auto const& errorCondition : errorConditions)
    {
      int callCount = 0;
      auto throwsNonTransient = [&callCount, errorCondition]() -> bool {
        ++callCount;
        throw LocalTest::MakeEventHubsException(errorCondition, "non-transient failure");
      };

      EXPECT_THROW(
          retryOp.Execute(throwsNonTransient, context),
          Azure::Messaging::EventHubs::EventHubsException);
      EXPECT_EQ(1, callCount) << errorCondition.ToString();
    }
  }

  TEST_F(RetryOperationTest, SucceedsAfterTransientException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto eventuallySucceeds = [&callCount]() -> bool {
      ++callCount;
      if (callCount == 1)
      {
        throw LocalTest::MakeEventHubsException(
            Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
            "first attempt fails");
      }
      return true;
    };

    EXPECT_TRUE(retryOp.Execute(eventuallySucceeds, context));
    EXPECT_EQ(2, callCount);
  }

  TEST_F(RetryOperationTest, FalseAfterTransientExceptionDoesNotRethrow)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto throwsThenReturnsFalse = [&callCount]() -> bool {
      ++callCount;
      if (callCount == 1)
      {
        throw LocalTest::MakeEventHubsException(
            Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
            "first attempt fails");
      }
      return false;
    };

    EXPECT_NO_THROW({ EXPECT_FALSE(retryOp.Execute(throwsThenReturnsFalse, context)); });
    EXPECT_EQ(opts.MaxRetries + 1, callCount);
  }

  TEST_F(RetryOperationTest, ZeroRetriesStillExecutesInitialAttempt)
  {
    auto opts = LocalTest::MakeFastRetryOptions(0);
    opts.RetryDelay = std::chrono::seconds(5);
    opts.MaxRetryDelay = std::chrono::seconds(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto throwsTransient = [&callCount]() -> bool {
      ++callCount;
      throw LocalTest::MakeEventHubsException(
          Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
          "initial attempt failed");
    };

    auto const attemptStart = std::chrono::steady_clock::now();
    EXPECT_THROW(
        retryOp.Execute(throwsTransient, context), Azure::Messaging::EventHubs::EventHubsException);
    auto const attemptTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - attemptStart);

    EXPECT_EQ(1, callCount);
    EXPECT_LT(attemptTime.count(), 1000);
  }

  TEST_F(RetryOperationTest, DoesNotRetryAuthenticationException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto throwsAuthenticationException = [&callCount]() -> bool {
      ++callCount;
      throw Azure::Core::Credentials::AuthenticationException("authentication failed");
    };

    EXPECT_THROW(
        retryOp.Execute(throwsAuthenticationException, context),
        Azure::Core::Credentials::AuthenticationException);
    EXPECT_EQ(1, callCount);
  }

  TEST_F(RetryOperationTest, DoesNotRetryOperationCancelledException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto throwsOperationCancelled = [&callCount]() -> bool {
      ++callCount;
      throw Azure::Core::OperationCancelledException("operation cancelled");
    };

    EXPECT_THROW(
        retryOp.Execute(throwsOperationCancelled, context),
        Azure::Core::OperationCancelledException);
    EXPECT_EQ(1, callCount);
  }

  TEST_F(RetryOperationTest, CancellationDuringOperationSurfacesOperationCancelledException)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    auto returnsCancelledAmqpError = [&]() -> bool {
      ++callCount;
      context.Cancel();
      throw LocalTest::MakeEventHubsException(
          Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::OperationCancelled,
          "message send operation cancelled");
    };

    EXPECT_THROW(
        retryOp.Execute(returnsCancelledAmqpError, context),
        Azure::Core::OperationCancelledException);
    EXPECT_EQ(1, callCount);
  }

  TEST_F(RetryOperationTest, SuccessfulOperationWinsCancellationRace)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    EXPECT_TRUE(retryOp.Execute(
        [&]() {
          ++callCount;
          context.Cancel();
          return true;
        },
        context));
    EXPECT_EQ(1, callCount);
  }

  TEST_F(RetryOperationTest, FailedOperationSurfacesCancellationBeforeRetry)
  {
    auto opts = LocalTest::MakeFastRetryOptions(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    int callCount = 0;

    EXPECT_THROW(
        retryOp.Execute(
            [&]() {
              ++callCount;
              context.Cancel();
              return false;
            },
            context),
        Azure::Core::OperationCancelledException);
    EXPECT_EQ(1, callCount);
  }

  TEST_F(RetryOperationTest, CancelledContextDoesNotInvokeOperation)
  {
    auto opts = LocalTest::MakeFastRetryOptions();
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    context.Cancel();
    int callCount = 0;

    EXPECT_THROW(
        retryOp.Execute(
            [&callCount]() {
              ++callCount;
              return true;
            },
            context),
        Azure::Core::OperationCancelledException);
    EXPECT_EQ(0, callCount);
  }

  TEST_F(RetryOperationTest, CancellingContextInterruptsBackoff)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    opts.RetryDelay = std::chrono::seconds(5);
    opts.MaxRetryDelay = std::chrono::seconds(5);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    std::atomic<int> callCount{0};
    std::promise<void> firstAttempt;
    auto firstAttemptFuture = firstAttempt.get_future();
    std::exception_ptr workerException;

    std::thread worker([&]() {
      try
      {
        retryOp.Execute(
            [&]() -> bool {
              auto const attempt = ++callCount;
              if (attempt == 1)
              {
                firstAttempt.set_value();
              }
              throw LocalTest::MakeEventHubsException(
                  Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError,
                  "transient failure");
            },
            context);
      }
      catch (...)
      {
        workerException = std::current_exception();
      }
    });

    if (firstAttemptFuture.wait_for(std::chrono::seconds(2)) != std::future_status::ready)
    {
      context.Cancel();
      worker.join();
      FAIL() << "The first retry attempt did not start.";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    auto const cancelStart = std::chrono::steady_clock::now();
    context.Cancel();
    worker.join();
    auto const cancellationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - cancelStart);

    EXPECT_LT(cancellationTime.count(), 1000);
    EXPECT_EQ(1, callCount.load());
    ASSERT_TRUE(workerException != nullptr);
    EXPECT_THROW(std::rethrow_exception(workerException), Azure::Core::OperationCancelledException);
  }

  // Every attempt writes the same warning, so an operator who reads the log
  // cannot tell attempt 1 from attempt 4. The warning must name the attempt.
  // The message "boom" holds no digit, so a digit in the line comes from the
  // attempt number.
  TEST_F(RetryOperationTest, RuntimeErrorWarningNamesTheAttempt)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    LocalTest::LogCapture logCapture;

    auto alwaysThrows = []() -> bool { throw std::runtime_error("boom"); };

    EXPECT_THROW(retryOp.Execute(alwaysThrows, context), std::runtime_error);

    auto const warnings = LocalTest::LinesContaining(
        logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning), "boom");
    ASSERT_EQ(static_cast<std::size_t>(opts.MaxRetries + 1), warnings.size());
    for (std::size_t i = 0; i < warnings.size(); ++i)
    {
      // The attempt number is 1-based, so the first attempt renders "1".
      EXPECT_NE(std::string::npos, warnings[i].find(std::to_string(i + 1))) << warnings[i];
    }
  }

  // The Event Hubs exception branch writes its own warning, and it has the same
  // problem. The condition "com.microsoft:timeout" holds no digit.
  TEST_F(RetryOperationTest, EventHubsExceptionWarningNamesTheAttempt)
  {
    auto opts = LocalTest::MakeFastRetryOptions(3);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    LocalTest::LogCapture logCapture;

    auto alwaysThrows = []() -> bool {
      throw LocalTest::MakeEventHubsException(
          Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::TimeoutError, "boom");
    };

    EXPECT_THROW(
        retryOp.Execute(alwaysThrows, context), Azure::Messaging::EventHubs::EventHubsException);

    auto const warnings = LocalTest::LinesContaining(
        logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning), "boom");
    ASSERT_EQ(static_cast<std::size_t>(opts.MaxRetries + 1), warnings.size());
    for (std::size_t i = 0; i < warnings.size(); ++i)
    {
      EXPECT_NE(std::string::npos, warnings[i].find(std::to_string(i + 1))) << warnings[i];
    }
  }

  // The line that says the retries are gone must say how many attempts ran and
  // what the limit was. 7 is not a count that appears for another reason here.
  TEST_F(RetryOperationTest, RetriesExhaustedNamesTheAttemptAndTheMaxRetries)
  {
    auto opts = LocalTest::MakeFastRetryOptions(7);
    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    Azure::Core::Context context;
    LocalTest::LogCapture logCapture;

    EXPECT_FALSE(retryOp.Execute([]() { return false; }, context));

    // Match a lower-case fragment, so a change of capitalisation does not break
    // this test.
    auto const exhausted = LocalTest::LinesContaining(
        logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Informational), "xhaust");
    ASSERT_EQ(static_cast<std::size_t>(1), exhausted.size());
    EXPECT_NE(std::string::npos, exhausted[0].find("7")) << exhausted[0];
    EXPECT_NE(std::string::npos, exhausted[0].find("8")) << exhausted[0];

    auto const warnings = LocalTest::LinesContaining(
        logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning), "xhaust");
    EXPECT_TRUE(warnings.empty());
  }

  TEST_F(RetryOperationTest, ConstructorCopiesRetryOptions)
  {
    auto opts = LocalTest::MakeFastRetryOptions(7);
    auto const expected = opts;

    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(opts);
    (void)retryOp;

    EXPECT_EQ(expected.MaxRetries, opts.MaxRetries);
    EXPECT_EQ(expected.RetryDelay, opts.RetryDelay);
    EXPECT_EQ(expected.MaxRetryDelay, opts.MaxRetryDelay);
    EXPECT_EQ(expected.StatusCodes, opts.StatusCodes);
  }
}}}}} // namespace Azure::Messaging::EventHubs::_internal::Test

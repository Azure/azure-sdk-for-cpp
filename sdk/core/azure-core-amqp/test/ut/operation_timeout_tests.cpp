// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/amqp/private/operation_timeout.hpp"
#include "../../src/amqp/private/pending_operations.hpp"
#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"

#if ENABLE_RUST_AMQP
#include "azure/core/amqp/internal/common/runtime_context.hpp"
#endif

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <tuple>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  // These tests pin the deadline rules that bound an AMQP operation, and the
  // registry that ends a pending operation when the connection goes away. The
  // rules are pure functions and the registry holds no transport, so these
  // tests run on every platform and need no service.
  class TestOperationTimeout : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  TEST_F(TestOperationTimeout, ARootContextCarriesNoDeadline)
  {
    Azure::Core::Context root;
    EXPECT_EQ(Azure::DateTime{(Azure::DateTime::max)()}, root.GetDeadline());
    EXPECT_FALSE(root.IsCancelled());
    EXPECT_FALSE(_detail::ContextHasDeadline(root));

    auto const child = root.WithDeadline(
        Azure::DateTime(std::chrono::system_clock::now() + std::chrono::seconds(30)));
    EXPECT_TRUE(_detail::ContextHasDeadline(child));
    EXPECT_FALSE(child.IsCancelled());
  }

  TEST_F(TestOperationTimeout, AContextWithNoDeadlineGetsTheDefaultBound)
  {
    auto const now = std::chrono::system_clock::now();

    auto const bounded = _detail::ContextWithOperationDeadline(Azure::Core::Context{}, now);
    EXPECT_EQ(Azure::DateTime(now + _detail::DefaultOperationTimeout), bounded.GetDeadline());
    EXPECT_FALSE(bounded.IsCancelled());
  }

  // GetDeadline takes the earliest deadline on the parent chain. A bound that
  // the code adds without a test of the caller's deadline therefore shortens a
  // caller deadline that is later than the default, which the second case here
  // catches.
  TEST_F(TestOperationTimeout, ACallerDeadlineWinsOverTheDefaultBound)
  {
    auto const now = std::chrono::system_clock::now();

    auto const soon = Azure::DateTime(now + std::chrono::seconds(5));
    auto const nearCaller = Azure::Core::Context{}.WithDeadline(soon);
    EXPECT_EQ(soon, _detail::ContextWithOperationDeadline(nearCaller, now).GetDeadline());

    auto const late = Azure::DateTime(now + std::chrono::hours(24));
    auto const farCaller = Azure::Core::Context{}.WithDeadline(late);
    EXPECT_EQ(late, _detail::ContextWithOperationDeadline(farCaller, now).GetDeadline());
  }

  TEST_F(TestOperationTimeout, ACancelledContextStaysCancelled)
  {
    auto const now = std::chrono::system_clock::now();

    Azure::Core::Context cancelled;
    cancelled.Cancel();
    EXPECT_TRUE(_detail::ContextHasDeadline(cancelled));

    auto const bounded = _detail::ContextWithOperationDeadline(cancelled, now);
    EXPECT_TRUE(bounded.IsCancelled());
  }

  TEST_F(TestOperationTimeout, TheBoundedContextEndsAWaitOnAnEmptyQueue)
  {
    // Put the deadline about 300 ms out without a dependency on the value of
    // DefaultOperationTimeout.
    auto const now = std::chrono::system_clock::now() - _detail::DefaultOperationTimeout
        + std::chrono::milliseconds(300);
    auto const bounded = _detail::ContextWithOperationDeadline(Azure::Core::Context{}, now);

    Common::_internal::AsyncOperationQueue<int> queue;
    std::promise<std::unique_ptr<std::tuple<int>>> waited;
    auto result = waited.get_future();

    auto const start = std::chrono::steady_clock::now();
    std::thread waiter(
        [&queue, &bounded, &waited]() { waited.set_value(queue.WaitForResult(bounded)); });

    if (result.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
      queue.CompleteOperation(0);
      waiter.join();
      FAIL() << "The bounded wait never ended.";
    }

    auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    auto const item = result.get();
    waiter.join();

    EXPECT_FALSE(item);
    EXPECT_GE(elapsed.count(), 200);
    EXPECT_LT(elapsed.count(), 5000);
  }

  class TestPendingOperations : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

#if ENABLE_UAMQP
  TEST_F(TestPendingOperations, AConnectionInErrorOrEndCannotCompleteAPendingOperation)
  {
    EXPECT_TRUE(_detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::Error));
    EXPECT_TRUE(_detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::End));

    EXPECT_FALSE(_detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::Start));
    EXPECT_FALSE(_detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::Opened));
    EXPECT_FALSE(
        _detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::HeaderExchanged));
    EXPECT_FALSE(
        _detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::CloseReceived));
    EXPECT_FALSE(
        _detail::ConnectionStateEndsPendingOperations(_internal::ConnectionState::Discarding));
  }
#endif // ENABLE_UAMQP

  TEST_F(TestPendingOperations, WakeAllGivesEveryRegisteredWaiterTheAmqpError)
  {
    _detail::PendingOperationRegistry registry;

    int firstCalls = 0;
    int secondCalls = 0;
    int thirdCalls = 0;
    Models::_internal::AmqpError firstSeen;
    Models::_internal::AmqpError thirdSeen;

    auto first
        = registry.Register([&firstCalls, &firstSeen](Models::_internal::AmqpError const& error) {
            ++firstCalls;
            firstSeen = error;
          });
    auto third
        = registry.Register([&thirdCalls, &thirdSeen](Models::_internal::AmqpError const& error) {
            ++thirdCalls;
            thirdSeen = error;
          });

    {
      auto second = registry.Register(
          [&secondCalls](Models::_internal::AmqpError const&) { ++secondCalls; });
      EXPECT_EQ(static_cast<std::size_t>(3), registry.PendingCount());
    }
    EXPECT_EQ(static_cast<std::size_t>(2), registry.PendingCount());

    Models::_internal::AmqpError error;
    error.Condition = Models::_internal::AmqpErrorCondition::ConnectionForced;
    error.Description = "The service closed the idle connection.";
    registry.WakeAll(error);

    EXPECT_EQ(1, firstCalls);
    EXPECT_EQ(0, secondCalls);
    EXPECT_EQ(1, thirdCalls);

    EXPECT_EQ(std::string("amqp:connection:forced"), firstSeen.Condition.ToString());
    EXPECT_EQ(error.Description, firstSeen.Description);
    EXPECT_EQ(std::string("amqp:connection:forced"), thirdSeen.Condition.ToString());
    EXPECT_EQ(error.Description, thirdSeen.Description);
  }

  TEST_F(TestPendingOperations, AWaiterBlockedOnTheQueueWakesWithTheConnectionError)
  {
    _detail::PendingOperationRegistry registry;
    auto queue
        = std::make_shared<Common::_internal::AsyncOperationQueue<Models::_internal::AmqpError>>();

    auto registration = registry.Register(
        [queue](Models::_internal::AmqpError const& error) { queue->CompleteOperation(error); });

    std::promise<std::unique_ptr<std::tuple<Models::_internal::AmqpError>>> waited;
    auto result = waited.get_future();
    std::thread waiter(
        [queue, &waited]() { waited.set_value(queue->WaitForResult(Azure::Core::Context{})); });

    // A default context never cancels, so the wait must still be running here.
    // Without that proof the wake below tells us nothing.
    EXPECT_EQ(std::future_status::timeout, result.wait_for(std::chrono::milliseconds(200)))
        << "The wait ended before the connection error arrived.";

    Models::_internal::AmqpError error;
    error.Condition = Models::_internal::AmqpErrorCondition::ConnectionForced;
    error.Description = "The service closed the idle connection.";
    registry.WakeAll(error);

    if (result.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
      queue->CompleteOperation(Models::_internal::AmqpError{});
      waiter.join();
      FAIL() << "The close did not wake the pending operation.";
    }

    auto const item = result.get();
    waiter.join();

    ASSERT_TRUE(item);
    EXPECT_EQ(std::string("amqp:connection:forced"), std::get<0>(*item).Condition.ToString());
    EXPECT_EQ(error.Description, std::get<0>(*item).Description);
  }

#if ENABLE_RUST_AMQP
  class TestRustCallContext : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  TEST_F(TestRustCallContext, ARootContextGetsTheDefaultBound)
  {
    Common::_detail::CallContext callContext(nullptr, Azure::Core::Context{});
    EXPECT_EQ(static_cast<std::uint64_t>(60000), callContext.GetTimeoutMilliseconds());
  }

  TEST_F(TestRustCallContext, ACallerDeadlineBecomesTheBound)
  {
    auto const caller = Azure::Core::Context{}.WithDeadline(
        Azure::DateTime(std::chrono::system_clock::now() + std::chrono::seconds(5)));

    Common::_detail::CallContext callContext(nullptr, caller);
    auto const timeout = callContext.GetTimeoutMilliseconds();
    EXPECT_GT(timeout, static_cast<std::uint64_t>(3000));
    EXPECT_LE(timeout, static_cast<std::uint64_t>(5000));
  }

  // A deadline in the past must give zero. The subtraction of two time points
  // is signed, so a cast of a negative result to an unsigned type gives a
  // duration of about 584 million years.
  TEST_F(TestRustCallContext, AnExpiredDeadlineGivesAZeroBound)
  {
    Azure::Core::Context cancelled;
    cancelled.Cancel();

    Common::_detail::CallContext callContext(nullptr, cancelled);
    EXPECT_EQ(static_cast<std::uint64_t>(0), callContext.GetTimeoutMilliseconds());
  }
#endif // ENABLE_RUST_AMQP

}}}} // namespace Azure::Core::Amqp::Tests

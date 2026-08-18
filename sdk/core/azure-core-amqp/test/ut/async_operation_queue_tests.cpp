// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <tuple>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Common::_internal;

class TestAsyncQueue : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestAsyncQueue, SimpleCreate)
{
  {
    AsyncOperationQueue<int> queue;
  }
}

TEST_F(TestAsyncQueue, InsertIntoQueue)
{
  {
    AsyncOperationQueue<int> queue;
    queue.CompleteOperation(25);
    std::unique_ptr<std::tuple<int>> item;
    Azure::Core::Context context;
    item = queue.WaitForResult(context);
    EXPECT_TRUE(item);
    EXPECT_EQ(25, std::get<0>(*item));
  }
}

TEST_F(TestAsyncQueue, CanceledContext)
{
  {
    AsyncOperationQueue<int> queue;
    std::unique_ptr<std::tuple<int>> item;
    Azure::Core::Context context;
    context.Cancel();
    item = queue.WaitForResult(context);
    EXPECT_FALSE(item);
  }
  {
    AsyncOperationQueue<int> queue;
    std::unique_ptr<std::tuple<int>> item;
    Azure::Core::Context context;
    context.Cancel();
    item = queue.WaitForPolledResult(context);
    EXPECT_FALSE(item);
  }
}

TEST_F(TestAsyncQueue, TryReadFromQueue)
{
  // Empty queue should return a null item.
  {
    AsyncOperationQueue<int> queue;
    std::unique_ptr<std::tuple<int>> item;
    item = queue.TryWaitForResult();
    EXPECT_FALSE(item);
  }

  // Peek item should return an item if it's in the queue.
  {
    AsyncOperationQueue<int> queue;
    queue.CompleteOperation(25);
    std::unique_ptr<std::tuple<int>> item;
    item = queue.TryWaitForResult();
    EXPECT_TRUE(item);
    EXPECT_EQ(25, std::get<0>(*item));
  }
}

TEST_F(TestAsyncQueue, ReadCanceled)
{
  {
    AsyncOperationQueue<int> queue;
    Azure::Core::Context context;
    context.Cancel();

    auto item = queue.WaitForResult(context);
    EXPECT_FALSE(item);
  }
}

// A deadline belongs to the caller's context, not to the queue. A bound inside
// WaitForResult would also bound the receiver long poll at
// message_receiver.cpp:197, which the caller controls. This test blocks that
// fix.
TEST_F(TestAsyncQueue, WaitForResultStillBlocksWithoutADeadline)
{
  AsyncOperationQueue<int> queue;
  std::promise<std::unique_ptr<std::tuple<int>>> waited;
  auto result = waited.get_future();

  std::thread waiter(
      [&queue, &waited]() { waited.set_value(queue.WaitForResult(Azure::Core::Context{})); });

  EXPECT_EQ(std::future_status::timeout, result.wait_for(std::chrono::milliseconds(500)));

  queue.CompleteOperation(42);
  auto const status = result.wait_for(std::chrono::seconds(5));
  waiter.join();

  EXPECT_EQ(std::future_status::ready, status);
  auto const item = result.get();
  ASSERT_TRUE(item);
  EXPECT_EQ(42, std::get<0>(*item));
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/common/async_operation_queue.hpp"

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
    Azure::Core::Context context;
    item = queue.TryWaitForResult();
    EXPECT_FALSE(item);
  }

  // Peek item should return an item if it's in the queue.
  {
    AsyncOperationQueue<int> queue;
    queue.CompleteOperation(25);
    std::unique_ptr<std::tuple<int>> item;
    Azure::Core::Context context;
    item = queue.TryWaitForResult();
    EXPECT_TRUE(item);
    EXPECT_EQ(25, std::get<0>(*item));
  }
}

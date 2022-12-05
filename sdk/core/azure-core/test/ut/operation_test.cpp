// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "operation_test.hpp"

#include <azure/core/context.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>

#include <chrono>

using namespace Azure::Core;
using namespace Azure::Core::Test;
using namespace std::literals;

TEST(Operation, Poll)
{
  StringClient client;
  auto operation = client.StartStringUpdate();

  EXPECT_FALSE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());

  while (!operation.IsDone())
  {
    EXPECT_FALSE(operation.HasValue());
    EXPECT_THROW(operation.Value(), std::runtime_error);
    auto response = operation.Poll();
  }

  EXPECT_TRUE(operation.IsDone());
  EXPECT_TRUE(operation.HasValue());

  auto result = operation.Value();
  EXPECT_EQ(result, "StringOperation-Completed");
}

TEST(Operation, PollUntilDone)
{
  StringClient client;
  auto operation = client.StartStringUpdate();

  EXPECT_FALSE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());
  EXPECT_THROW(operation.Value(), std::runtime_error);

  auto start = std::chrono::high_resolution_clock::now();
  auto response = operation.PollUntilDone(500ms);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  // StringOperation test code is implemented to poll 2 times
  EXPECT_TRUE(elapsed >= 1s);

  EXPECT_TRUE(operation.IsDone());
  EXPECT_TRUE(operation.HasValue());

  auto result = operation.Value();
  EXPECT_EQ(result, "StringOperation-Completed");
}

TEST(Operation, Status)
{
  StringClient client;
  auto operation = client.StartStringUpdate();

  EXPECT_FALSE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());
  EXPECT_THROW(operation.Value(), std::runtime_error);
  EXPECT_EQ(operation.Status(), OperationStatus::NotStarted);

  operation.SetOperationStatus(OperationStatus::Running);
  EXPECT_FALSE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());
  EXPECT_THROW(operation.Value(), std::runtime_error);
  EXPECT_EQ(operation.Status(), OperationStatus::Running);

  operation.SetOperationStatus(OperationStatus::Failed);
  EXPECT_TRUE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());
  EXPECT_THROW(operation.Value(), std::runtime_error);
  EXPECT_EQ(operation.Status(), OperationStatus::Failed);

  operation.SetOperationStatus(OperationStatus::Cancelled);
  EXPECT_TRUE(operation.IsDone());
  EXPECT_FALSE(operation.HasValue());
  EXPECT_THROW(operation.Value(), std::runtime_error);
  EXPECT_EQ(operation.Status(), OperationStatus::Cancelled);
}

TEST(Operation, ResumeToken)
{
  StringClient client;
  std::string token;
  {
    auto operation = client.StartStringUpdate();
    token = operation.GetResumeToken();
  }
  {
    for (auto resumedOperation = StringOperation::CreateFromResumeToken(token, client);
         !resumedOperation.IsDone();
         resumedOperation.Poll())
    {
      EXPECT_FALSE(resumedOperation.HasValue());
      EXPECT_THROW(resumedOperation.Value(), std::runtime_error);
    }
  }
}
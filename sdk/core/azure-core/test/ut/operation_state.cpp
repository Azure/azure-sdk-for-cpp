// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/operation_state.hpp>

using namespace Azure::Core;

TEST(OperationState, Basic)
{
  OperationState status = OperationState::Cancelled;
  EXPECT_EQ(status, OperationState::Cancelled);
  EXPECT_EQ(status.Get(), "Cancelled");

  status = OperationState::Failed;
  EXPECT_EQ(status, OperationState::Failed);
  EXPECT_EQ(status.Get(), "Failed");

  status = OperationState::NotStarted;
  EXPECT_EQ(status, OperationState::NotStarted);
  EXPECT_EQ(status.Get(), "NotStarted");

  status = OperationState::Running;
  EXPECT_EQ(status, OperationState::Running);
  EXPECT_EQ(status.Get(), "Running");

  status = OperationState::Succeeded;
  EXPECT_EQ(status, OperationState::Succeeded);
  EXPECT_EQ(status.Get(), "Succeeded");
}

TEST(OperationState, Custom)
{
  OperationState status1("CustomValue");
  EXPECT_EQ(status1.Get(), "CustomValue");
  EXPECT_NE(status1, OperationState::NotStarted);

  OperationState status2 = OperationState("CustomValue");
  EXPECT_EQ(status2.Get(), "CustomValue");
  EXPECT_NE(status2, OperationState::NotStarted);

  std::string custom("CustomValue");
  OperationState status3 = OperationState(custom);
  EXPECT_EQ(status3.Get(), custom);
  EXPECT_NE(status3, OperationState::NotStarted);

  OperationState status4 = OperationState(std::string("CustomValue"));
  EXPECT_EQ(status4.Get(), "CustomValue");
  EXPECT_NE(status4, OperationState::NotStarted);
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/operation_status.hpp>

using namespace Azure::Core;

TEST(OperationStatus, Basic)
{
  OperationStatus status = OperationStatus::Cancelled;
  EXPECT_EQ(status, OperationStatus::Cancelled);
  EXPECT_EQ(status.Get(), "Cancelled");

  status = OperationStatus::Failed;
  EXPECT_EQ(status, OperationStatus::Failed);
  EXPECT_EQ(status.Get(), "Failed");

  status = OperationStatus::NotStarted;
  EXPECT_EQ(status, OperationStatus::NotStarted);
  EXPECT_EQ(status.Get(), "NotStarted");

  status = OperationStatus::Running;
  EXPECT_EQ(status, OperationStatus::Running);
  EXPECT_EQ(status.Get(), "Running");

  status = OperationStatus::Succeeded;
  EXPECT_EQ(status, OperationStatus::Succeeded);
  EXPECT_EQ(status.Get(), "Succeeded");
}

TEST(OperationStatus, Custom)
{
  OperationStatus status1("CustomValue");
  EXPECT_EQ(status1.Get(), "CustomValue");
  EXPECT_NE(status1, OperationStatus::NotStarted);

  OperationStatus status2 = OperationStatus("CustomValue");
  EXPECT_EQ(status2.Get(), "CustomValue");
  EXPECT_NE(status2, OperationStatus::NotStarted);

  std::string custom("CustomValue");
  OperationStatus status3 = OperationStatus(custom);
  EXPECT_EQ(status3.Get(), custom);
  EXPECT_NE(status3, OperationStatus::NotStarted);

  OperationStatus status4 = OperationStatus(std::string("CustomValue"));
  EXPECT_EQ(status4.Get(), "CustomValue");
  EXPECT_NE(status4, OperationStatus::NotStarted);

  EXPECT_EQ(status1, status2);
  EXPECT_EQ(status2, status3);
  EXPECT_EQ(status3, status4);
}
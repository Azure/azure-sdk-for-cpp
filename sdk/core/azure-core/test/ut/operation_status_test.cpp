// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_traits.hpp"

#include <azure/core/operation_status.hpp>

#include <gtest/gtest.h>

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

TEST(OperationStatus, Assignable)
{
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_assignable<OperationStatus>());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_assignable<const OperationStatus>());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_assignable<OperationStatus>());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_assignable<const OperationStatus>());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_nothrow_assignable<OperationStatus>());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_nothrow_assignable<const OperationStatus>());
}

TEST(OperationStatus, Constructible)
{
  EXPECT_TRUE((ClassTraits<OperationStatus, const std::string&>::is_constructible()));
  EXPECT_FALSE((ClassTraits<OperationStatus, const std::string&>::is_trivially_constructible()));
  EXPECT_FALSE((ClassTraits<OperationStatus, const std::string&>::is_nothrow_constructible()));
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_default_constructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_default_constructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_nothrow_default_constructible());
}

TEST(OperationStatus, CopyAndMoveConstructible)
{
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_copy_constructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_copy_constructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_nothrow_copy_constructible());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_move_constructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_move_constructible());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_nothrow_move_constructible());
}

TEST(OperationStatus, CopyAndMoveAssignable)
{
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_copy_assignable());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_copy_assignable());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_nothrow_copy_assignable());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_move_assignable());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_move_assignable());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_nothrow_move_assignable());
}

TEST(OperationStatus, Destructible)
{
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_destructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::is_trivially_destructible());
  EXPECT_TRUE(ClassTraits<OperationStatus>::is_nothrow_destructible());
  EXPECT_FALSE(ClassTraits<OperationStatus>::has_virtual_destructor());
}


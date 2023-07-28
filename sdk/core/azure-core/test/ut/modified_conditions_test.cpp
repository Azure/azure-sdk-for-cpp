// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_traits.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/modified_conditions.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

using namespace Azure;

TEST(ModifiedConditions, Basic)
{
  ModifiedConditions conditions;
  conditions.IfModifiedSince
      = DateTime::Parse("2013-11-19T14:30:59.1234567Z", DateTime::DateFormat::Rfc3339);
  conditions.IfUnmodifiedSince
      = DateTime::Parse("2013-11-19T14:30:59.1234567Z", DateTime::DateFormat::Rfc3339);

  EXPECT_EQ(
      conditions.IfModifiedSince->ToString(DateTime::DateFormat::Rfc3339),
      "2013-11-19T14:30:59.1234567Z");
  EXPECT_EQ(
      conditions.IfUnmodifiedSince->ToString(DateTime::DateFormat::Rfc3339),
      "2013-11-19T14:30:59.1234567Z");
}

TEST(ModifiedConditions, Constructible)
{
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_constructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_default_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_default_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_default_constructible());
}

TEST(ModifiedConditions, Destructible)
{
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_destructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_destructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_nothrow_destructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::has_virtual_destructor());
}

TEST(ModifiedConditions, CopyAndMoveConstructible)
{
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_copy_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_copy_constructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_nothrow_copy_constructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_move_constructible());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_move_constructible());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_nothrow_move_constructible());
}

TEST(ModifiedConditions, Assignable)
{
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_assignable<ModifiedConditions>());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_assignable<const ModifiedConditions>());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_assignable<ModifiedConditions>());
  EXPECT_FALSE(
      ClassTraits<ModifiedConditions>::is_trivially_assignable<const ModifiedConditions>());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_assignable<ModifiedConditions>());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_assignable<const ModifiedConditions>());
}

TEST(ModifiedConditions, CopyAndMoveAssignable)
{
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_copy_assignable());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_copy_assignable());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_copy_assignable());
  EXPECT_TRUE(ClassTraits<ModifiedConditions>::is_move_assignable());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_trivially_move_assignable());
  EXPECT_FALSE(ClassTraits<ModifiedConditions>::is_nothrow_move_assignable());
}

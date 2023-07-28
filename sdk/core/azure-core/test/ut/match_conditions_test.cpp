// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_traits.hpp"

#include <azure/core/etag.hpp>
#include <azure/core/match_conditions.hpp>

#include <string>

#include <gtest/gtest.h>

using namespace Azure;

TEST(MatchConditions, Basic)
{
  MatchConditions match;
  match.IfMatch = ETag("IfMatch");
  match.IfNoneMatch = ETag("IfNoneMatch");

  EXPECT_EQ(match.IfMatch.ToString(), "IfMatch");
  EXPECT_EQ(match.IfNoneMatch.ToString(), "IfNoneMatch");
}

TEST(MatchConditions, Constructible)
{
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_constructible());
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_default_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_default_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_default_constructible());
}

TEST(MatchConditions, Destructible)
{
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_destructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_destructible());
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_nothrow_destructible());
  EXPECT_TRUE(ClassTraits<MatchConditions>::has_virtual_destructor());
}

TEST(MatchConditions, CopyAndMoveConstructible)
{
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_copy_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_copy_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_copy_constructible());
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_move_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_move_constructible());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_move_constructible());
}

TEST(MatchConditions, Assignable)
{
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_assignable<MatchConditions>());
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_assignable<const MatchConditions>());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_assignable<MatchConditions>());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_assignable<const MatchConditions>());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_assignable<MatchConditions>());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_assignable<const MatchConditions>());
}

TEST(MatchConditions, CopyAndMoveAssignable)
{
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_copy_assignable());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_copy_assignable());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_copy_assignable());
  EXPECT_TRUE(ClassTraits<MatchConditions>::is_move_assignable());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_trivially_move_assignable());
  EXPECT_FALSE(ClassTraits<MatchConditions>::is_nothrow_move_assignable());
}

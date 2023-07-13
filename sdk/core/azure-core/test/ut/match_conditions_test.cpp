// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/etag.hpp>
#include <azure/core/match_conditions.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Azure;

TEST(MatchConditions, Basic)
{
  MatchConditions match;
  match.IfMatch = ETag("IfMatch");
  match.IfNoneMatch = ETag("IfNoneMatch");

  EXPECT_EQ(match.IfMatch.ToString(), "IfMatch");
  EXPECT_EQ(match.IfNoneMatch.ToString(), "IfNoneMatch");
}

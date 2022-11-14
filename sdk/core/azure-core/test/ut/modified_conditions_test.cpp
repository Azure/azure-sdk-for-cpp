//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/datetime.hpp>
#include <azure/core/modified_conditions.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

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

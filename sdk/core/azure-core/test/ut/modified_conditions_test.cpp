// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

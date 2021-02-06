// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/request_conditions.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(RequestConditions, Basic)
{
  RequestConditions conditions;
  conditions.IfMatch = ETag("IfMatch");
  conditions.IfNoneMatch = ETag("IfNoneMatch");
  conditions.IfModifiedSince
      = DateTime::Parse("2013-11-19T14:30:59.1234567Z", DateTime::DateFormat::Rfc3339);
  conditions.IfUnmodifiedSince
      = DateTime::Parse("2013-11-19T14:30:59.1234567Z", DateTime::DateFormat::Rfc3339);

  EXPECT_EQ(conditions.IfMatch.ToString(), "IfMatch");
  EXPECT_EQ(conditions.IfNoneMatch.ToString(), "IfNoneMatch");

  EXPECT_EQ(
      conditions.IfModifiedSince.GetString(DateTime::DateFormat::Rfc3339),
      "2013-11-19T14:30:59.1234567Z");
  EXPECT_EQ(
      conditions.IfUnmodifiedSince.GetString(DateTime::DateFormat::Rfc3339),
      "2013-11-19T14:30:59.1234567Z");
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core.hpp>

#include <limits>

using namespace Azure::Core;

TEST(Headers, SimpleInclude)
{
  //This test exists to check compilation of core.hpp

  auto dt1 = DateTime::FromString("20130517T00:00:00Z", DateTime::DateFormat::Iso8601);
  EXPECT_NE(0u, dt1.ToInterval());

  Nullable<std::string> testString{"hello world"};
  EXPECT_TRUE(testString.HasValue());

  Context context;  
  auto& valueT = context["key"];
  EXPECT_FALSE(context.HasKey(""));
  EXPECT_FALSE(context.HasKey("key"));

  auto uuid = Uuid::CreateUuid();
  EXPECT_TRUE(uuid.GetUuidString().length() == 36);

}

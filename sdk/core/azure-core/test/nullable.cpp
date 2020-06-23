// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <nullable.hpp>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Nullable, basic)
{
    Nullable<std::string> testString{"hello world"};
    EXPECT_TRUE(testString.HasValue());
    EXPECT_TRUE(testString.GetValue() == "hello world");

    Nullable<int> testInt{54321};
    EXPECT_TRUE(testInt.HasValue());
    EXPECT_TRUE(testInt.GetValue() == 54321);

    Nullable<double> testDouble{10.0};
    EXPECT_TRUE(testDouble.HasValue());
    EXPECT_TRUE(testDouble.GetValue() == 10.0);
}

TEST(Nullable, empty)
{
  Nullable<std::string> testString{};
  EXPECT_FALSE(testString.HasValue());

  Nullable<int> testInt{};
  EXPECT_FALSE(testInt.HasValue());

  Nullable<double> testDouble{};
  EXPECT_FALSE(testDouble.HasValue());
}

TEST(Nullable, assignment)
{
  Nullable<std::string> instance{"hello world"};
  auto instance2 = instance;
  EXPECT_TRUE(instance2.HasValue());
  EXPECT_TRUE(instance2.GetValue() == "hello world");

  auto instance3 = std::move(instance);
  EXPECT_TRUE(instance3.HasValue());
  EXPECT_TRUE(instance3.GetValue() == "hello world");

  EXPECT_FALSE(instance.HasValue());
}


// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "context.hpp"

#include <string>
#include <vector>
#include <chrono>
#include <memory>

using namespace Azure::Core;

TEST(Context, Basic)
{
  Context context;
  auto& valueT1 = context["key"];
  EXPECT_FALSE(context.HasKey(""));
  EXPECT_FALSE(context.HasKey("key"));
}

TEST(Context, BasicBool) {
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", true);
  auto& valueT = c2["key"];
  auto value = valueT.Get<bool>();
  EXPECT_TRUE(value == true);
}

TEST(Context, BasicInt)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto& valueT = c2["key"];
  auto value = valueT.Get<int>();
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", s);
  auto& valueT = c2["key"];
  auto value = valueT.Get<std::string>();
  EXPECT_TRUE(value == s);
}

TEST(Context, Alternative) {

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto& valueT1 = c2["key"];
  auto& valueT2 = c2["otherKey"];

  EXPECT_TRUE(valueT1.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(valueT2.Alternative() == ContextValue::ContextValueType::Undefined);
}

TEST(Context, Chain)
{

  Context context;
  // New context from previous
  auto c2 = context.WithValue("c2", 123);
  auto c3 = c2.WithValue("c3", 456);
  auto c4 = c3.WithValue("c4", 789);
  auto c5 = c4.WithValue("c5", "5");
  auto c6 = c5.WithValue("c6", "6");
  auto c7 = c6.WithValue("c7", "7");
  auto finalContext = c7.WithValue("finalContext", "Final");
  
  auto& valueT2 = finalContext["c2"];
  auto& valueT3 = finalContext["c3"];
  auto& valueT4 = finalContext["c4"];
  auto& valueT5 = finalContext["c5"];
  auto& valueT6 = finalContext["c6"];
  auto& valueT7 = finalContext["c7"];
  auto& valueT8 = finalContext["finalContext"];
  auto& valueT9 = finalContext["otherKey"];

  EXPECT_TRUE(valueT2.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(valueT3.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(valueT4.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(valueT5.Alternative() == ContextValue::ContextValueType::StdString);
  EXPECT_TRUE(valueT6.Alternative() == ContextValue::ContextValueType::StdString);
  EXPECT_TRUE(valueT7.Alternative() == ContextValue::ContextValueType::StdString);
  EXPECT_TRUE(valueT8.Alternative() == ContextValue::ContextValueType::StdString);
  EXPECT_TRUE(valueT9.Alternative() == ContextValue::ContextValueType::Undefined);

  auto value = valueT2.Get<int>();
  EXPECT_TRUE(value == 123);
  value = valueT3.Get<int>();
  EXPECT_TRUE(value == 456);
  value = valueT4.Get<int>();
  EXPECT_TRUE(value == 789);

  auto str = valueT5.Get<std::string>();
  EXPECT_TRUE(str == "5");
  str = valueT6.Get<std::string>();
  EXPECT_TRUE(str == "6");
  str = valueT7.Get<std::string>();
  EXPECT_TRUE(str == "7");

  str = valueT8.Get<std::string>();
  EXPECT_TRUE(str == "Final");
}


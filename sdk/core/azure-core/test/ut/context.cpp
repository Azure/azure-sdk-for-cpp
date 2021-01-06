// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/context.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace Azure::Core;

TEST(Context, Basic)
{
  Context context;
  auto& valueT = context["key"];
  EXPECT_FALSE(context.HasKey(""));
  EXPECT_FALSE(context.HasKey("key"));

  auto kind = valueT.Alternative();
  EXPECT_TRUE(kind == ContextValue::ContextValueType::Undefined);
}

TEST(Context, BasicBool)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", true);
  auto& valueT = c2["key"];
  auto value = valueT.Get<bool>();
  EXPECT_TRUE(value == true);

  auto kind = valueT.Alternative();
  EXPECT_TRUE(kind == ContextValue::ContextValueType::Bool);
}

TEST(Context, BasicInt)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto& valueT = c2["key"];
  auto value = valueT.Get<int>();
  EXPECT_TRUE(value == 123);

  auto kind = valueT.Alternative();
  EXPECT_TRUE(kind == ContextValue::ContextValueType::Int);
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

  auto kind = valueT.Alternative();
  EXPECT_TRUE(kind == ContextValue::ContextValueType::StdString);
}

TEST(Context, BasicChar)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", str);
  auto& valueT = c2["key"];
  auto value = valueT.Get<std::string>();
  EXPECT_TRUE(value == s);

  auto kind = valueT.Alternative();
  EXPECT_TRUE(kind == ContextValue::ContextValueType::StdString);
}

TEST(Context, IsCancelled)
{
  auto duration = std::chrono::milliseconds(150);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.WithDeadline(deadline);
  EXPECT_FALSE(c2.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_TRUE(c2.IsCancelled());
}

TEST(Context, Alternative)
{
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

TEST(Context, MatchingKeys)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto c3 = c2.WithValue("key", 456);

  auto& valueT2 = c2["key"];
  auto& valueT3 = c3["key"];
  auto& missing = c3["otherKey"];

  EXPECT_TRUE(valueT2.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(valueT3.Alternative() == ContextValue::ContextValueType::Int);
  EXPECT_TRUE(missing.Alternative() == ContextValue::ContextValueType::Undefined);

  auto value = valueT2.Get<int>();
  EXPECT_TRUE(value == 123);
  value = valueT3.Get<int>();
  EXPECT_TRUE(value == 456);
}

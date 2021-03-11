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
  EXPECT_FALSE(context.HasKey(""));
  EXPECT_FALSE(context.HasKey("key"));
}

TEST(Context, BasicBool)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", true);
  auto& value = c2.Get<bool>("key");
  EXPECT_TRUE(value == true);
}

TEST(Context, BasicInt)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto& value = c2.Get<int>("key");
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", s);
  auto& value = c2.Get<std::string>("key");
  EXPECT_TRUE(value == s);
}

TEST(Context, BasicChar)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", str);
  auto& value = c2.Get<const char*>("key");
  EXPECT_TRUE(value == s);
  EXPECT_TRUE(value == str);
}

TEST(Context, IsCancelled)
{
  auto duration = std::chrono::milliseconds(250);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.WithDeadline(deadline);
  EXPECT_FALSE(c2.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_TRUE(c2.IsCancelled());
}

TEST(Context, NestedIsCancelled)
{
  auto duration = std::chrono::milliseconds(250);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.WithValue("Key", "Value");
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey("Key"));
  EXPECT_FALSE(context.HasKey("Key"));

  auto c3 = context.WithDeadline(deadline);
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_FALSE(c3.IsCancelled());
  std::this_thread::sleep_for(duration);

  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c3.IsCancelled());

  EXPECT_TRUE(c2.HasKey("Key"));
  EXPECT_FALSE(context.HasKey("Key"));
  EXPECT_FALSE(c3.HasKey("Key"));
}

TEST(Context, CancelWithValue)
{
  Context context;
  auto c2 = context.WithValue("Key", "Value");
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey("Key"));
  EXPECT_FALSE(context.HasKey("Key"));

  c2.Cancel();
  EXPECT_TRUE(c2.IsCancelled());
  EXPECT_FALSE(context.IsCancelled());

  EXPECT_TRUE(c2.HasKey("Key"));
  EXPECT_FALSE(context.HasKey("Key"));
}

TEST(Context, ThrowIfCancelled)
{
  auto duration = std::chrono::milliseconds(250);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.WithDeadline(deadline);
  EXPECT_NO_THROW(c2.ThrowIfCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_THROW(c2.ThrowIfCancelled(), Azure::Core::OperationCancelledException);
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

  auto& valueT2 = finalContext.Get<int>("c2");
  auto& valueT3 = finalContext.Get<int>("c3");
  auto& valueT4 = finalContext.Get<int>("c4");
  auto& valueT5 = finalContext.Get<const char*>("c5");
  auto& valueT6 = finalContext.Get<const char*>("c6");
  auto& valueT7 = finalContext.Get<const char*>("c7");
  auto& valueT8 = finalContext.Get<const char*>("finalContext");

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
  EXPECT_TRUE(valueT4 == 789);
  EXPECT_TRUE(valueT5 == std::string("5"));
  EXPECT_TRUE(valueT6 == std::string("6"));
  EXPECT_TRUE(valueT7 == std::string("7"));
  EXPECT_TRUE(valueT8 == std::string("Final"));
}

TEST(Context, MatchingKeys)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", 123);
  auto c3 = c2.WithValue("key", 456);

  auto& valueT2 = c2.Get<int>("key");
  auto& valueT3 = c3.Get<int>("key");

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
}

struct SomeStructForContext
{
  int someField = 12345;
};

TEST(Context, InstanceValue)
{
  auto contextP = Context::GetApplicationContext().WithValue("struct", SomeStructForContext());
  auto& contextValueRef = contextP.Get<SomeStructForContext>("struct");
  EXPECT_EQ(contextValueRef.someField, 12345);
}

TEST(Context, UniquePtr)
{
  auto contextP = Context::GetApplicationContext().WithValue(
      "struct", std::make_unique<SomeStructForContext>());
  auto& contextValueRef = contextP.Get<std::unique_ptr<SomeStructForContext>>("struct");
  EXPECT_EQ(contextValueRef->someField, 12345);
}

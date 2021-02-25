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
  auto valueT = context["key"];
  EXPECT_FALSE(context.HasKey(""));
  EXPECT_FALSE(context.HasKey("key"));
  EXPECT_EQ(valueT, nullptr);
}

TEST(Context, BasicBool)
{
  Context context;
  // New context from previous
  bool value(true);
  auto c2 = context.WithValue("key", reinterpret_cast<void*>(&value));
  auto valueVoidP = c2["key"];
  auto valueT = *reinterpret_cast<bool*>(valueVoidP);
  EXPECT_EQ(value, valueT);
}

TEST(Context, BasicInt)
{
  Context context;
  int value = 123;
  // New context from previous
  auto c2 = context.WithValue("key", reinterpret_cast<void*>(&value));
  auto valueVoidP = c2["key"];
  auto valueT = *reinterpret_cast<int*>(valueVoidP);
  EXPECT_EQ(value, valueT);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");
  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", reinterpret_cast<void*>(&s));
  auto valueT = c2["key"];
  auto value = *reinterpret_cast<std::string*>(valueT);
  EXPECT_EQ(value, s);
}

TEST(Context, BasicChar)
{
  char str[] = "Test String";

  Context context;
  // New context from previous
  auto c2 = context.WithValue("key", reinterpret_cast<void*>(str));
  auto valueT = c2["key"];
  auto value = reinterpret_cast<char*>(valueT);
  EXPECT_EQ(value, str);
}

TEST(Context, ApplicationContext)
{
  Context appContext = GetApplicationContext();

  EXPECT_FALSE(appContext.HasKey("Key"));
  EXPECT_FALSE(appContext.HasKey("key"));
  EXPECT_FALSE(appContext.HasKey("Value"));
  EXPECT_FALSE(appContext.HasKey("value"));
  EXPECT_FALSE(appContext.HasKey("1"));
  EXPECT_FALSE(appContext.HasKey(""));

  auto duration = std::chrono::milliseconds(250);
  EXPECT_FALSE(appContext.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_FALSE(appContext.IsCancelled());

  appContext.Cancel();
  EXPECT_TRUE(appContext.IsCancelled());

  // AppContext2 is the same context as AppContext
  //  The context should be cancelled
  Context appContext2 = GetApplicationContext();
  EXPECT_TRUE(appContext.IsCancelled());
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
  std::string valueForContext("Value");

  Context context;
  auto c2 = context.WithValue("Key", reinterpret_cast<void*>(&valueForContext));
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
  std::string valueForContext("Value");
  auto c2 = context.WithValue("Key", reinterpret_cast<void*>(&valueForContext));
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
  int value2 = 123;
  int value3 = 456;
  int value4 = 789;
  char value5 = '5';
  char value6 = '6';
  char value7 = '7';
  std::string final("Final");
  // New context from previous
  auto c2 = context.WithValue("c2", reinterpret_cast<void*>(&value2));
  auto c3 = c2.WithValue("c3", reinterpret_cast<void*>(&value3));
  auto c4 = c3.WithValue("c4", reinterpret_cast<void*>(&value4));
  auto c5 = c4.WithValue("c5", reinterpret_cast<void*>(&value5));
  auto c6 = c5.WithValue("c6", reinterpret_cast<void*>(&value6));
  auto c7 = c6.WithValue("c7", reinterpret_cast<void*>(&value7));
  auto finalContext = c7.WithValue("finalContext", reinterpret_cast<void*>(&final));

  auto valueT2 = finalContext["c2"];
  auto valueT3 = finalContext["c3"];
  auto valueT4 = finalContext["c4"];
  auto valueT5 = finalContext["c5"];
  auto valueT6 = finalContext["c6"];
  auto valueT7 = finalContext["c7"];
  auto valueT8 = finalContext["finalContext"];
  auto valueT9 = finalContext["otherKey"];

  auto value = reinterpret_cast<int*>(valueT2);
  EXPECT_EQ(*value, value2);
  value = reinterpret_cast<int*>(valueT3);
  EXPECT_EQ(*value, value3);
  value = reinterpret_cast<int*>(valueT4);
  EXPECT_EQ(*value, value4);

  auto str = reinterpret_cast<char*>(valueT5);
  EXPECT_EQ(*str, value5);
  str = reinterpret_cast<char*>(valueT6);
  EXPECT_EQ(*str, value6);
  str = reinterpret_cast<char*>(valueT7);
  EXPECT_EQ(*str, value7);

  auto valueT = *reinterpret_cast<std::string*>(valueT8);
  EXPECT_EQ(valueT, final);

  EXPECT_EQ(valueT9, nullptr);
}

TEST(Context, MatchingKeys)
{
  Context context;
  int value2 = 123;
  int value3 = 456;
  // New context from previous
  auto c2 = context.WithValue("key", reinterpret_cast<void*>(&value2));
  auto c3 = c2.WithValue("key", reinterpret_cast<void*>(&value3));

  auto valueT2 = c2["key"];
  auto valueT3 = c3["key"];
  auto missing = c3["otherKey"];

  auto valueT = *reinterpret_cast<int*>(valueT2);
  EXPECT_EQ(valueT, value2);
  valueT = *reinterpret_cast<int*>(valueT3);
  EXPECT_EQ(valueT, value3);
  EXPECT_EQ(missing, nullptr);
}

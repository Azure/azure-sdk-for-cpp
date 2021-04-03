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
  auto c2 = context.CreateChildContext("key", true);
  auto& value = c2.GetValue<bool>("key");
  EXPECT_TRUE(value == true);
}

TEST(Context, BasicInt)
{
  Context context;
  // New context from previous
  auto c2 = context.CreateChildContext("key", 123);
  auto& value = c2.GetValue<int>("key");
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  // New context from previous
  auto c2 = context.CreateChildContext("key", s);
  auto& value = c2.GetValue<std::string>("key");
  EXPECT_TRUE(value == s);
}

TEST(Context, BasicChar)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  // New context from previous
  auto c2 = context.CreateChildContext("key", str);
  auto& value = c2.GetValue<const char*>("key");
  EXPECT_TRUE(value == s);
  EXPECT_TRUE(value == str);
}

TEST(Context, IsCancelled)
{
  auto duration = std::chrono::milliseconds(250);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.CreateChildContext(deadline);
  EXPECT_FALSE(c2.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_TRUE(c2.IsCancelled());
}

TEST(Context, NestedIsCancelled)
{
  auto duration = std::chrono::milliseconds(250);
  auto deadline = std::chrono::system_clock::now() + duration;

  Context context;
  auto c2 = context.CreateChildContext("Key", "Value");
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey("Key"));
  EXPECT_FALSE(context.HasKey("Key"));

  auto c3 = context.CreateChildContext(deadline);
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
  auto c2 = context.CreateChildContext("Key", "Value");
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
  auto c2 = context.CreateChildContext(deadline);
  EXPECT_NO_THROW(c2.ThrowIfCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_THROW(c2.ThrowIfCancelled(), Azure::Core::OperationCancelledException);
}

TEST(Context, Chain)
{
  Context context;
  // New context from previous
  auto c2 = context.CreateChildContext("c2", 123);
  auto c3 = c2.CreateChildContext("c3", 456);
  auto c4 = c3.CreateChildContext("c4", 789);
  auto c5 = c4.CreateChildContext("c5", "5");
  auto c6 = c5.CreateChildContext("c6", "6");
  auto c7 = c6.CreateChildContext("c7", "7");
  auto finalContext = c7.CreateChildContext("finalContext", "Final");

  auto& valueT2 = finalContext.GetValue<int>("c2");
  auto& valueT3 = finalContext.GetValue<int>("c3");
  auto& valueT4 = finalContext.GetValue<int>("c4");
  auto& valueT5 = finalContext.GetValue<const char*>("c5");
  auto& valueT6 = finalContext.GetValue<const char*>("c6");
  auto& valueT7 = finalContext.GetValue<const char*>("c7");
  auto& valueT8 = finalContext.GetValue<const char*>("finalContext");

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
  auto c2 = context.CreateChildContext("key", 123);
  auto c3 = c2.CreateChildContext("key", 456);

  auto& valueT2 = c2.GetValue<int>("key");
  auto& valueT3 = c3.GetValue<int>("key");

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
}

struct SomeStructForContext
{
  int someField = 12345;
};

TEST(Context, InstanceValue)
{
  auto contextP
      = Context::GetApplicationContext().CreateChildContext("struct", SomeStructForContext());
  auto& contextValueRef = contextP.GetValue<SomeStructForContext>("struct");
  EXPECT_EQ(contextValueRef.someField, 12345);
}

TEST(Context, UniquePtr)
{
  auto contextP = Context::GetApplicationContext().CreateChildContext(
      "struct", std::make_unique<SomeStructForContext>());
  auto& contextValueRef = contextP.GetValue<std::unique_ptr<SomeStructForContext>>("struct");
  EXPECT_EQ(contextValueRef->someField, 12345);
}

TEST(Context, HeapLinkIntegrity)
{
  Context thirdGeneration; // To be used at the end
  {
    Context root;
    auto firstGeneration = root.CreateChildContext("a", std::string("a"));
    EXPECT_TRUE(firstGeneration.HasKey("a"));

    auto secondGeneration = firstGeneration.CreateChildContext("b", std::string("b"));
    EXPECT_TRUE(secondGeneration.HasKey("a"));
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey("b"));
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>("b"));

    // Now overide the generation
    secondGeneration = secondGeneration.CreateChildContext("c", std::string("c"));
    EXPECT_TRUE(
        secondGeneration.HasKey("a")); // Still know about first gen - The link is still in heap
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey(
        "b")); // Still knows about the initial second gen, as a shared_ptr, it is still on heap
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>("b"));
    EXPECT_TRUE(secondGeneration.HasKey("c")); // Check new value
    EXPECT_EQ("c", secondGeneration.GetValue<std::string>("c"));

    // One more override
    secondGeneration = secondGeneration.CreateChildContext("d", std::string("d"));
    EXPECT_TRUE(secondGeneration.HasKey("a"));
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey("b"));
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>("b"));
    EXPECT_TRUE(secondGeneration.HasKey("c"));
    EXPECT_EQ("c", secondGeneration.GetValue<std::string>("c"));
    EXPECT_TRUE(secondGeneration.HasKey("d"));
    EXPECT_EQ("d", secondGeneration.GetValue<std::string>("d"));

    // New Gen
    thirdGeneration = secondGeneration.CreateChildContext("e", std::string("e"));
  }
  // Went out of scope, root and secondGeneration are destroyed. but should remain in heap for the
  // third-generation since the previous geneations are still alive inside his heart <3.
  EXPECT_TRUE(thirdGeneration.HasKey("a"));
  EXPECT_EQ("a", thirdGeneration.GetValue<std::string>("a"));
  EXPECT_TRUE(thirdGeneration.HasKey("b"));
  EXPECT_EQ("b", thirdGeneration.GetValue<std::string>("b"));
  EXPECT_TRUE(thirdGeneration.HasKey("c"));
  EXPECT_EQ("c", thirdGeneration.GetValue<std::string>("c"));
  EXPECT_TRUE(thirdGeneration.HasKey("d"));
  EXPECT_EQ("d", thirdGeneration.GetValue<std::string>("d"));
  EXPECT_TRUE(thirdGeneration.HasKey("e"));
  EXPECT_EQ("e", thirdGeneration.GetValue<std::string>("e"));
}

TEST(Context, Deadline)
{
  auto const deadline = Azure::DateTime(2021, 4, 1, 23, 45, 15);
  {
    Context ctx;
    EXPECT_FALSE(ctx.HasDeadline());
    EXPECT_EQ(ctx.GetDeadline(), Azure::DateTime::max());

    ctx.Cancel();
    EXPECT_TRUE(ctx.HasDeadline());
    EXPECT_EQ(ctx.GetDeadline(), Azure::DateTime::min());
  }

  {
    Context ctx;
    ctx = ctx.CreateChildContext(deadline);

    EXPECT_TRUE(ctx.HasDeadline());
    EXPECT_EQ(ctx.GetDeadline(), deadline);
  }

  {
    Context ctx;

    auto childCtx = ctx.CreateChildContext(deadline)
                        .CreateChildContext("key", "val")
                        .CreateChildContext("key2", "val2");

    EXPECT_TRUE(childCtx.HasDeadline());
    EXPECT_EQ(childCtx.GetDeadline(), deadline);
  }

  {
    Context ctx;
    ctx.Cancel();

    auto childCtx = ctx.CreateChildContext(deadline)
                        .CreateChildContext("key", "val")
                        .CreateChildContext("key2", "val2");

    EXPECT_TRUE(childCtx.HasDeadline());
    EXPECT_EQ(childCtx.GetDeadline(), Azure::DateTime::min());
  }
}

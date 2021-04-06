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
  Context::Key const key;

  EXPECT_FALSE(context.HasKey(key));
}

TEST(Context, BasicBool)
{
  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, true);
  auto& value = c2.GetValue<bool>(key);
  EXPECT_TRUE(value == true);
}

TEST(Context, BasicInt)
{
  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, 123);
  auto& value = c2.GetValue<int>(key);
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, s);
  auto& value = c2.GetValue<std::string>(key);
  EXPECT_TRUE(value == s);
}

TEST(Context, BasicChar)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, str);
  auto& value = c2.GetValue<const char*>(key);
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
  Context::Key const key;

  auto c2 = context.WithValue(key, "Value");
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey(key));
  EXPECT_FALSE(context.HasKey(key));

  auto c3 = context.WithDeadline(deadline);
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_FALSE(c3.IsCancelled());
  std::this_thread::sleep_for(duration);

  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c3.IsCancelled());

  EXPECT_TRUE(c2.HasKey(key));
  EXPECT_FALSE(context.HasKey(key));
  EXPECT_FALSE(c3.HasKey(key));
}

TEST(Context, CancelWithValue)
{
  Context context;
  Context::Key const key;

  auto c2 = context.WithValue(key, "Value");
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey(key));
  EXPECT_FALSE(context.HasKey(key));

  c2.Cancel();
  EXPECT_TRUE(c2.IsCancelled());
  EXPECT_FALSE(context.IsCancelled());

  EXPECT_TRUE(c2.HasKey(key));
  EXPECT_FALSE(context.HasKey(key));
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
  Context::Key const key2;
  Context::Key const key3;
  Context::Key const key4;
  Context::Key const key5;
  Context::Key const key6;
  Context::Key const key7;
  Context::Key const keyFinal;

  // New context from previous
  auto c2 = context.WithValue(key2, 123);
  auto c3 = c2.WithValue(key3, 456);
  auto c4 = c3.WithValue(key4, 789);
  auto c5 = c4.WithValue(key5, "5");
  auto c6 = c5.WithValue(key6, "6");
  auto c7 = c6.WithValue(key7, "7");
  auto finalContext = c7.WithValue(keyFinal, "Final");

  auto& valueT2 = finalContext.GetValue<int>(key2);
  auto& valueT3 = finalContext.GetValue<int>(key3);
  auto& valueT4 = finalContext.GetValue<int>(key4);
  auto& valueT5 = finalContext.GetValue<const char*>(key5);
  auto& valueT6 = finalContext.GetValue<const char*>(key6);
  auto& valueT7 = finalContext.GetValue<const char*>(key7);
  auto& valueT8 = finalContext.GetValue<const char*>(keyFinal);

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
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, 123);
  auto c3 = c2.WithValue(key, 456);

  auto& valueT2 = c2.GetValue<int>(key);
  auto& valueT3 = c3.GetValue<int>(key);

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
}

struct SomeStructForContext
{
  int someField = 12345;
};

TEST(Context, InstanceValue)
{
  Context::Key const key;
  auto contextP = Context::GetApplicationContext().WithValue(key, SomeStructForContext());
  auto& contextValueRef = contextP.GetValue<SomeStructForContext>(key);
  EXPECT_EQ(contextValueRef.someField, 12345);
}

TEST(Context, UniquePtr)
{
  Context::Key const key;
  auto contextP
      = Context::GetApplicationContext().WithValue(key, std::make_unique<SomeStructForContext>());
  auto& contextValueRef = contextP.GetValue<std::unique_ptr<SomeStructForContext>>(key);
  EXPECT_EQ(contextValueRef->someField, 12345);
}

TEST(Context, HeapLinkIntegrity)
{
  Context::Key const a;
  Context::Key const b;
  Context::Key const c;
  Context::Key const d;
  Context::Key const e;
  Context thirdGeneration; // To be used at the end
  {
    Context root;
    auto firstGeneration = root.WithValue(a, std::string("a"));
    EXPECT_TRUE(firstGeneration.HasKey(a));

    auto secondGeneration = firstGeneration.WithValue(b, std::string("b"));
    EXPECT_TRUE(secondGeneration.HasKey(a));
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>(a));
    EXPECT_TRUE(secondGeneration.HasKey(b));
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>(b));

    // Now overide the generation
    secondGeneration = secondGeneration.WithValue(c, std::string("c"));
    EXPECT_TRUE(
        secondGeneration.HasKey(a)); // Still know about first gen - The link is still in heap
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>(a));
    EXPECT_TRUE(secondGeneration.HasKey(
        b)); // Still knows about the initial second gen, as a shared_ptr, it is still on heap
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>(b));
    EXPECT_TRUE(secondGeneration.HasKey(c)); // Check new value
    EXPECT_EQ("c", secondGeneration.GetValue<std::string>(c));

    // One more override
    secondGeneration = secondGeneration.WithValue(d, std::string("d"));
    EXPECT_TRUE(secondGeneration.HasKey(a));
    EXPECT_EQ("a", secondGeneration.GetValue<std::string>(a));
    EXPECT_TRUE(secondGeneration.HasKey(b));
    EXPECT_EQ("b", secondGeneration.GetValue<std::string>(b));
    EXPECT_TRUE(secondGeneration.HasKey(c));
    EXPECT_EQ("c", secondGeneration.GetValue<std::string>(c));
    EXPECT_TRUE(secondGeneration.HasKey(d));
    EXPECT_EQ("d", secondGeneration.GetValue<std::string>(d));

    // New Gen
    thirdGeneration = secondGeneration.WithValue(e, std::string("e"));
  }
  // Went out of scope, root and secondGeneration are destroyed. but should remain in heap for the
  // third-generation since the previous geneations are still alive inside his heart <3.
  EXPECT_TRUE(thirdGeneration.HasKey(a));
  EXPECT_EQ("a", thirdGeneration.GetValue<std::string>(a));
  EXPECT_TRUE(thirdGeneration.HasKey(b));
  EXPECT_EQ("b", thirdGeneration.GetValue<std::string>(b));
  EXPECT_TRUE(thirdGeneration.HasKey(c));
  EXPECT_EQ("c", thirdGeneration.GetValue<std::string>(c));
  EXPECT_TRUE(thirdGeneration.HasKey(d));
  EXPECT_EQ("d", thirdGeneration.GetValue<std::string>(d));
  EXPECT_TRUE(thirdGeneration.HasKey(e));
  EXPECT_EQ("e", thirdGeneration.GetValue<std::string>(e));
}

Context::Key const GlobalKey1;
Context::Key const GlobalKey2;

namespace {
Context::Key const UnnamedNamespaceKey1;
Context::Key const UnnamedNamespaceKey2;
} // namespace

TEST(Context, KeyComparison)
{
  EXPECT_EQ(GlobalKey1, GlobalKey1);
  EXPECT_EQ(GlobalKey2, GlobalKey2);

  EXPECT_NE(GlobalKey1, GlobalKey2);
  EXPECT_NE(GlobalKey2, GlobalKey1);

  EXPECT_EQ(UnnamedNamespaceKey1, UnnamedNamespaceKey1);
  EXPECT_EQ(UnnamedNamespaceKey2, UnnamedNamespaceKey2);

  EXPECT_NE(UnnamedNamespaceKey1, UnnamedNamespaceKey2);
  EXPECT_NE(UnnamedNamespaceKey2, UnnamedNamespaceKey1);

  Context::Key const localKey1;
  Context::Key const localKey2;

  EXPECT_EQ(localKey1, localKey1);
  EXPECT_EQ(localKey2, localKey2);

  EXPECT_NE(localKey1, localKey2);
  EXPECT_NE(localKey2, localKey1);

  Context::Key const localKey1Copy = localKey1;
  Context::Key const localKey2Copy = localKey2;

  EXPECT_EQ(localKey1Copy, localKey1Copy);
  EXPECT_EQ(localKey2Copy, localKey2Copy);

  EXPECT_NE(localKey1Copy, localKey2Copy);
  EXPECT_NE(localKey2Copy, localKey1Copy);

  EXPECT_EQ(localKey1, localKey1Copy);
  EXPECT_EQ(localKey2, localKey2Copy);
  EXPECT_EQ(localKey1Copy, localKey1);
  EXPECT_EQ(localKey2Copy, localKey2);

  EXPECT_NE(localKey1, localKey2Copy);
  EXPECT_NE(localKey2, localKey1Copy);
  EXPECT_NE(localKey1Copy, localKey2);
  EXPECT_NE(localKey2Copy, localKey1);
}

TEST(Context, Deadline)
{
  auto const deadline = Azure::DateTime(2021, 4, 1, 23, 45, 15);
  Context::Key const key1;
  Context::Key const key2;

  {
    Context ctx;
    EXPECT_EQ(ctx.GetDeadline(), Azure::DateTime::max());

    ctx.Cancel();
    EXPECT_EQ(ctx.GetDeadline(), Azure::DateTime::min());
  }

  {
    Context ctx;
    ctx = ctx.WithDeadline(deadline);
    EXPECT_EQ(ctx.GetDeadline(), deadline);
  }

  {
    Context ctx;
    auto childCtx = ctx.WithDeadline(deadline).WithValue(key1, "val").WithValue(key2, "val2");
    EXPECT_EQ(childCtx.GetDeadline(), deadline);
  }

  {
    Context ctx;
    ctx.Cancel();

    auto childCtx = ctx.WithDeadline(deadline).WithValue(key1, "val").WithValue(key2, "val2");

    EXPECT_EQ(childCtx.GetDeadline(), Azure::DateTime::min());
  }
}

TEST(Context, ValueOr)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  Context::Key const key;

  auto value = context.ValueOr<std::string>(key, str);

  EXPECT_TRUE(value == s);
  EXPECT_TRUE(value == str);
}

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

namespace {
extern char const KeyId[] = "key";
extern char const Key2Id[] = "key2";
extern char const Key3Id[] = "key3";
extern char const Key4Id[] = "key4";
extern char const Key5Id[] = "key5";
extern char const Key6Id[] = "key6";
extern char const Key7Id[] = "key7";
extern char const KeyFinalId[] = "keyFinal";

Context::Key const Key = _internal::ContextKey::Create<KeyId>();
Context::Key const Key2 = _internal::ContextKey::Create<Key2Id>();
Context::Key const Key3 = _internal::ContextKey::Create<Key3Id>();
Context::Key const Key4 = _internal::ContextKey::Create<Key4Id>();
Context::Key const Key5 = _internal::ContextKey::Create<Key5Id>();
Context::Key const Key6 = _internal::ContextKey::Create<Key6Id>();
Context::Key const Key7 = _internal::ContextKey::Create<Key7Id>();
Context::Key const KeyFinal = _internal::ContextKey::Create<KeyFinalId>();
} // namespace

TEST(Context, Basic)
{
  Context context;
  EXPECT_FALSE(context.HasKey(Key));
}

TEST(Context, BasicBool)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue(Key, true);
  auto& value = c2.Get<bool>(Key);
  EXPECT_TRUE(value == true);
}

TEST(Context, BasicInt)
{
  Context context;
  // New context from previous
  auto c2 = context.WithValue(Key, 123);
  auto& value = c2.Get<int>(Key);
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  // New context from previous
  auto c2 = context.WithValue(Key, s);
  auto& value = c2.Get<std::string>(Key);
  EXPECT_TRUE(value == s);
}

TEST(Context, BasicChar)
{
  const char* str = "Test String";
  std::string s(str);

  Context context;
  // New context from previous
  auto c2 = context.WithValue(Key, str);
  auto& value = c2.Get<const char*>(Key);
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
  auto c2 = context.WithValue(Key, "Value");
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey(Key));
  EXPECT_FALSE(context.HasKey(Key));

  auto c3 = context.WithDeadline(deadline);
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_FALSE(c3.IsCancelled());
  std::this_thread::sleep_for(duration);

  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c3.IsCancelled());

  EXPECT_TRUE(c2.HasKey(Key));
  EXPECT_FALSE(context.HasKey(Key));
  EXPECT_FALSE(c3.HasKey(Key));
}

TEST(Context, CancelWithValue)
{
  Context context;
  auto c2 = context.WithValue(Key, "Value");
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c2.HasKey(Key));
  EXPECT_FALSE(context.HasKey(Key));

  c2.Cancel();
  EXPECT_TRUE(c2.IsCancelled());
  EXPECT_FALSE(context.IsCancelled());

  EXPECT_TRUE(c2.HasKey(Key));
  EXPECT_FALSE(context.HasKey(Key));
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
  auto c2 = context.WithValue(Key2, 123);
  auto c3 = c2.WithValue(Key3, 456);
  auto c4 = c3.WithValue(Key4, 789);
  auto c5 = c4.WithValue(Key5, "5");
  auto c6 = c5.WithValue(Key6, "6");
  auto c7 = c6.WithValue(Key7, "7");
  auto finalContext = c7.WithValue(KeyFinal, "Final");

  auto& valueT2 = finalContext.Get<int>(Key2);
  auto& valueT3 = finalContext.Get<int>(Key3);
  auto& valueT4 = finalContext.Get<int>(Key4);
  auto& valueT5 = finalContext.Get<const char*>(Key5);
  auto& valueT6 = finalContext.Get<const char*>(Key6);
  auto& valueT7 = finalContext.Get<const char*>(Key7);
  auto& valueT8 = finalContext.Get<const char*>(KeyFinal);

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
  auto c2 = context.WithValue(Key, 123);
  auto c3 = c2.WithValue(Key, 456);

  auto& valueT2 = c2.Get<int>(Key);
  auto& valueT3 = c3.Get<int>(Key);

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
}

struct SomeStructForContext
{
  int someField = 12345;
};

TEST(Context, InstanceValue)
{
  auto contextP = Context::GetApplicationContext().WithValue(Key, SomeStructForContext());
  auto& contextValueRef = contextP.Get<SomeStructForContext>(Key);
  EXPECT_EQ(contextValueRef.someField, 12345);
}

TEST(Context, UniquePtr)
{
  auto contextP
      = Context::GetApplicationContext().WithValue(Key, std::make_unique<SomeStructForContext>());
  auto& contextValueRef = contextP.Get<std::unique_ptr<SomeStructForContext>>(Key);
  EXPECT_EQ(contextValueRef->someField, 12345);
}

TEST(Context, HeapLinkIntegrity)
{
  Context thirdGeneration; // To be used at the end
  {
    Context root;
    auto firstGeneration = root.WithValue("a", std::string("a"));
    EXPECT_TRUE(firstGeneration.HasKey("a"));

    auto secondGeneration = firstGeneration.WithValue("b", std::string("b"));
    EXPECT_TRUE(secondGeneration.HasKey("a"));
    EXPECT_EQ("a", secondGeneration.Get<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey("b"));
    EXPECT_EQ("b", secondGeneration.Get<std::string>("b"));

    // Now overide the generation
    secondGeneration = secondGeneration.WithValue("c", std::string("c"));
    EXPECT_TRUE(
        secondGeneration.HasKey("a")); // Still know about first gen - The link is still in heap
    EXPECT_EQ("a", secondGeneration.Get<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey(
        "b")); // Still knows about the initial second gen, as a shared_ptr, it is still on heap
    EXPECT_EQ("b", secondGeneration.Get<std::string>("b"));
    EXPECT_TRUE(secondGeneration.HasKey("c")); // Check new value
    EXPECT_EQ("c", secondGeneration.Get<std::string>("c"));

    // One more override
    secondGeneration = secondGeneration.WithValue("d", std::string("d"));
    EXPECT_TRUE(secondGeneration.HasKey("a"));
    EXPECT_EQ("a", secondGeneration.Get<std::string>("a"));
    EXPECT_TRUE(secondGeneration.HasKey("b"));
    EXPECT_EQ("b", secondGeneration.Get<std::string>("b"));
    EXPECT_TRUE(secondGeneration.HasKey("c"));
    EXPECT_EQ("c", secondGeneration.Get<std::string>("c"));
    EXPECT_TRUE(secondGeneration.HasKey("d"));
    EXPECT_EQ("d", secondGeneration.Get<std::string>("d"));

    // New Gen
    thirdGeneration = secondGeneration.WithValue("e", std::string("e"));
  }
  // Went out of scope, root and secondGeneration are destroyed. but should remain in heap for the
  // third-generation since the previous geneations are still alive inside his heart <3.
  EXPECT_TRUE(thirdGeneration.HasKey("a"));
  EXPECT_EQ("a", thirdGeneration.Get<std::string>("a"));
  EXPECT_TRUE(thirdGeneration.HasKey("b"));
  EXPECT_EQ("b", thirdGeneration.Get<std::string>("b"));
  EXPECT_TRUE(thirdGeneration.HasKey("c"));
  EXPECT_EQ("c", thirdGeneration.Get<std::string>("c"));
  EXPECT_TRUE(thirdGeneration.HasKey("d"));
  EXPECT_EQ("d", thirdGeneration.Get<std::string>("d"));
  EXPECT_TRUE(thirdGeneration.HasKey("e"));
  EXPECT_EQ("e", thirdGeneration.Get<std::string>("e"));
}

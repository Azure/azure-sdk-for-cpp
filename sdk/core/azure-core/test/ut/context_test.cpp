//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/context.hpp>
#include <azure/core/tracing/tracing.hpp>

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

  int placeholder = -15;
  EXPECT_FALSE(context.TryGetValue(key, placeholder));
  EXPECT_EQ(placeholder, -15);
}

TEST(Context, BasicBool)
{
  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, true);
  bool value{};
  EXPECT_TRUE(c2.TryGetValue<bool>(key, value));
  EXPECT_TRUE(value == true);

  Context::Key const anotherKey;
  auto c3 = c2.WithValue(anotherKey, std::make_shared<bool>(true));

  std::shared_ptr<bool> sharedPtrBool;
  EXPECT_FALSE(c2.TryGetValue<std::shared_ptr<bool>>(anotherKey, sharedPtrBool));
  EXPECT_FALSE(sharedPtrBool);

  EXPECT_TRUE(c3.TryGetValue(anotherKey, sharedPtrBool));
  EXPECT_TRUE(sharedPtrBool);
  EXPECT_TRUE(*sharedPtrBool);
}

TEST(Context, BasicInt)
{
  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, 123);
  int value;
  EXPECT_TRUE(c2.TryGetValue<int>(key, value));
  EXPECT_TRUE(value == 123);
}

TEST(Context, BasicStdString)
{
  std::string s("Test String");

  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, s);
  std::string value;
  EXPECT_TRUE(c2.TryGetValue<std::string>(key, value));
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
  const char* value;
  EXPECT_TRUE(c2.TryGetValue<const char*>(key, value));
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

  std::string actualValue = "Value";
  auto c2 = context.WithValue(key, actualValue);
  EXPECT_FALSE(c2.IsCancelled());
  std::string value = "a";
  EXPECT_TRUE(c2.TryGetValue<std::string>(key, value));
  EXPECT_EQ(value, "Value");
  value = "temp";
  EXPECT_FALSE(context.TryGetValue<std::string>(key, value));
  EXPECT_EQ(value, "temp");

  auto c3 = context.WithDeadline(deadline);
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_FALSE(c3.IsCancelled());
  std::this_thread::sleep_for(duration);

  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  EXPECT_TRUE(c3.IsCancelled());

  value = "b";
  EXPECT_TRUE(c2.TryGetValue<std::string>(key, value));
  EXPECT_EQ(value, "Value");
  EXPECT_FALSE(context.TryGetValue<std::string>(key, value));
  EXPECT_FALSE(c3.TryGetValue<std::string>(key, value));
}

TEST(Context, CancelWithValue)
{
  Context context;
  Context::Key const key;

  std::string actualValue = "Value";
  auto c2 = context.WithValue(key, actualValue);
  EXPECT_FALSE(context.IsCancelled());
  EXPECT_FALSE(c2.IsCancelled());
  std::string value = "a";
  EXPECT_TRUE(c2.TryGetValue<std::string>(key, value));
  EXPECT_EQ(value, "Value");
  EXPECT_FALSE(context.TryGetValue<std::string>(key, value));

  c2.Cancel();
  EXPECT_TRUE(c2.IsCancelled());
  EXPECT_FALSE(context.IsCancelled());

  EXPECT_TRUE(c2.TryGetValue<std::string>(key, value));
  EXPECT_EQ(value, "Value");
  EXPECT_FALSE(context.TryGetValue<std::string>(key, value));
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

  int valueT2;
  EXPECT_TRUE(finalContext.TryGetValue<int>(key2, valueT2));
  int valueT3;
  EXPECT_TRUE(finalContext.TryGetValue<int>(key3, valueT3));
  int valueT4;
  EXPECT_TRUE(finalContext.TryGetValue<int>(key4, valueT4));
  const char* valueT5;
  EXPECT_TRUE(finalContext.TryGetValue<const char*>(key5, valueT5));
  const char* valueT6;
  EXPECT_TRUE(finalContext.TryGetValue<const char*>(key6, valueT6));
  const char* valueT7;
  EXPECT_TRUE(finalContext.TryGetValue<const char*>(key7, valueT7));
  const char* valueT8;
  EXPECT_TRUE(finalContext.TryGetValue<const char*>(keyFinal, valueT8));

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

  int valueT2;
  EXPECT_TRUE(c2.TryGetValue<int>(key, valueT2));
  int valueT3;
  EXPECT_TRUE(c3.TryGetValue<int>(key, valueT3));

  EXPECT_TRUE(valueT2 == 123);
  EXPECT_TRUE(valueT3 == 456);
}

struct SomeStructForContext final
{
  int someField = 12345;
};

TEST(Context, InstanceValue)
{
  Context::Key const key;
  auto contextP = Context::ApplicationContext.WithValue(key, SomeStructForContext());
  SomeStructForContext contextValueRef;
  EXPECT_TRUE(contextP.TryGetValue<SomeStructForContext>(key, contextValueRef));
  EXPECT_EQ(contextValueRef.someField, 12345);
}

TEST(Context, Ptr)
{
  Context::Key const key;
  SomeStructForContext value;
  auto contextP = Context::ApplicationContext.WithValue(key, &value);

  SomeStructForContext* contextValueRef;
  EXPECT_TRUE(contextP.TryGetValue<SomeStructForContext*>(key, contextValueRef));
  EXPECT_EQ(contextValueRef->someField, 12345);
  EXPECT_EQ(&value, contextValueRef);
}

TEST(Context, NestedClassPtr)
{
  class TestClass final {
  private:
    int* m_instanceCount;

  public:
    TestClass(int* instanceCount) : m_instanceCount(instanceCount) { ++(*m_instanceCount); }
    ~TestClass() { --(*m_instanceCount); }
  };

  int instanceCount = 0;
  {
    auto sharedPtr = std::make_shared<TestClass>(&instanceCount);
    EXPECT_EQ(sharedPtr.use_count(), 1);

    Context::Key const key;

    auto context = Context::ApplicationContext.WithValue(key, sharedPtr);
    EXPECT_EQ(sharedPtr.use_count(), 2);

    std::shared_ptr<TestClass> foundPtr;
    EXPECT_TRUE(context.TryGetValue(key, foundPtr));
    EXPECT_EQ(foundPtr.get(), sharedPtr.get());
    EXPECT_EQ(instanceCount, 1);
    EXPECT_EQ(sharedPtr.use_count(), 3);
  }

  // Verify that context calls the destructor of shared_ptr it is holding
  EXPECT_EQ(instanceCount, 0);
}

TEST(Context, HeapLinkIntegrity)
{
  std::string value = "z";
  Context::Key const a;
  Context::Key const b;
  Context::Key const c;
  Context::Key const d;
  Context::Key const e;
  Context thirdGeneration; // To be used at the end
  {
    Context root;
    auto firstGeneration = root.WithValue(a, std::string("a"));
    EXPECT_TRUE(firstGeneration.TryGetValue<std::string>(a, value));
    EXPECT_EQ(value, "a");

    auto secondGeneration = firstGeneration.WithValue(b, std::string("b"));
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(a, value));
    EXPECT_EQ(value, "a");
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(b, value));
    EXPECT_EQ(value, "b");

    // Now overide the generation
    secondGeneration = secondGeneration.WithValue(c, std::string("c"));
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(a, value));
    EXPECT_EQ(value, "a"); // Still know about first gen - The link is still in heap
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(b, value));
    EXPECT_EQ(
        value,
        "b"); // Still knows about the initial second gen, as a shared_ptr, it is still on heap
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(c, value));
    EXPECT_EQ(value, "c"); // Check new value

    // One more override
    secondGeneration = secondGeneration.WithValue(d, std::string("d"));
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(a, value));
    EXPECT_EQ(value, "a");
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(b, value));
    EXPECT_EQ(value, "b");
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(c, value));
    EXPECT_EQ(value, "c");
    EXPECT_TRUE(secondGeneration.TryGetValue<std::string>(d, value));
    EXPECT_EQ(value, "d");

    // New Gen
    thirdGeneration = secondGeneration.WithValue(e, std::string("e"));
  }
  // Went out of scope, root and secondGeneration are destroyed. but should remain in heap for the
  // third-generation since the previous geneations are still alive inside his heart <3.
  EXPECT_TRUE(thirdGeneration.TryGetValue<std::string>(a, value));
  EXPECT_EQ(value, "a");
  EXPECT_TRUE(thirdGeneration.TryGetValue<std::string>(b, value));
  EXPECT_EQ(value, "b");
  EXPECT_TRUE(thirdGeneration.TryGetValue<std::string>(c, value));
  EXPECT_EQ(value, "c");
  EXPECT_TRUE(thirdGeneration.TryGetValue<std::string>(d, value));
  EXPECT_EQ(value, "d");
  EXPECT_TRUE(thirdGeneration.TryGetValue<std::string>(e, value));
  EXPECT_EQ(value, "e");
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

#if defined(AZ_CORE_RTTI) && GTEST_HAS_DEATH_TEST
TEST(Context, PreCondition)
{
  // Get a mismatch type from the context
  std::string s("Test String");

  Context context;
  Context::Key const key;

  // New context from previous
  auto c2 = context.WithValue(key, s);
  int value;

// Type-safe assert requires RTTI build
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(c2.TryGetValue<int>(key, value), "");
#else
  ASSERT_DEATH(c2.TryGetValue<int>(key, value), "Type mismatch for Context::TryGetValue");
#endif
}
#endif

TEST(Context, KeyTypePairPrecondition)
{
  Context context;
  Context::Key const key;
  Context::Key const keyNotFound;

  std::string s("Test String");

  // New context from previous
  auto c2 = context.WithValue(key, 123);
  auto c3 = c2.WithValue(key, s);

  int intValue = -1;
  std::string strValue = "previous value";

  EXPECT_FALSE(c2.TryGetValue<std::string>(keyNotFound, strValue));
  EXPECT_FALSE(c2.TryGetValue<int>(keyNotFound, intValue));

#if GTEST_HAS_DEATH_TEST
// Type-safe assert requires RTTI build
#if defined(AZ_CORE_RTTI)
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(c2.TryGetValue<std::string>(key, strValue), "");
#else
  ASSERT_DEATH(
      c2.TryGetValue<std::string>(key, strValue), "Type mismatch for Context::TryGetValue");
#endif
#endif
#endif

  EXPECT_TRUE(strValue == "previous value");

  EXPECT_TRUE(c2.TryGetValue<int>(key, intValue));
  EXPECT_TRUE(intValue == 123);

#if GTEST_HAS_DEATH_TEST
// Type-safe assert requires RTTI build
#if defined(AZ_CORE_RTTI)
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(c3.TryGetValue<int>(key, intValue), "");
#else
  ASSERT_DEATH(c3.TryGetValue<int>(key, intValue), "Type mismatch for Context::TryGetValue");
#endif
#endif
#endif

  EXPECT_TRUE(intValue == 123);

  EXPECT_TRUE(c3.TryGetValue<std::string>(key, strValue));
  EXPECT_TRUE(strValue == s);
}

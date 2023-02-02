// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/nullable.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace Azure;

TEST(Nullable, Basic)
{
  Nullable<std::string> testString{"hello world"};
  EXPECT_TRUE(testString.HasValue());
  EXPECT_TRUE(testString.Value() == "hello world");

  Nullable<int> testInt{54321};
  EXPECT_TRUE(testInt.HasValue());
  EXPECT_TRUE(testInt.Value() == 54321);

  Nullable<double> testDouble{10.0};
  EXPECT_TRUE(testDouble.HasValue());
  EXPECT_TRUE(testDouble.Value() == 10.0);
}

TEST(Nullable, Empty)
{
  Nullable<std::string> testString{};
  EXPECT_FALSE(testString.HasValue());
  EXPECT_TRUE(!testString);

  Nullable<std::string> testString2;
  EXPECT_FALSE(testString2.HasValue());
  EXPECT_TRUE(!testString2);

  Nullable<int> testInt{};
  EXPECT_FALSE(testInt.HasValue());
  EXPECT_TRUE(!testInt);

  Nullable<int> testInt2;
  EXPECT_FALSE(testInt2.HasValue());
  EXPECT_TRUE(!testInt2);

  Nullable<double> testDouble{};
  EXPECT_FALSE(testDouble.HasValue());
  EXPECT_TRUE(!testDouble);

  Nullable<double> testDouble2;
  EXPECT_FALSE(testDouble2.HasValue());
  EXPECT_TRUE(!testDouble2);
}

TEST(Nullable, Assignment)
{
  Nullable<std::string> instance{"hello world"};
  auto instance2 = instance;
  EXPECT_TRUE(instance2.HasValue());
  EXPECT_TRUE(instance2.Value() == "hello world");

  auto instance3 = std::move(instance);
  EXPECT_TRUE(instance3.HasValue());
  EXPECT_TRUE(instance3.Value() == "hello world");

  EXPECT_TRUE(instance.HasValue());

  // This is not a guarantee that the string will be empty
  //  It is an implementation detail that the contents are moved
  //  Should a future compiler change this assumption this test will need updates
  EXPECT_TRUE(instance.Value() == "");
  EXPECT_TRUE(instance.HasValue());
}

TEST(Nullable, ValueAssignment)
{
  Nullable<int> intVal;
  EXPECT_FALSE(intVal.HasValue());
  intVal = 7;
  EXPECT_TRUE(intVal.HasValue());
  EXPECT_TRUE(intVal.Value() == 7);

  Nullable<double> doubleVal;
  EXPECT_FALSE(doubleVal.HasValue());
  doubleVal = 10.12345;
  EXPECT_TRUE(doubleVal.HasValue());
  EXPECT_TRUE(doubleVal.Value() == 10.12345);

  Nullable<std::string> strVal;
  EXPECT_FALSE(strVal.HasValue());
  strVal = std::string("Hello World");
  EXPECT_TRUE(strVal.HasValue());
  EXPECT_TRUE(strVal.Value() == "Hello World");

  strVal = "New String";
  EXPECT_TRUE(strVal.Value() == "New String");

  strVal.Reset();
  EXPECT_FALSE(strVal.HasValue());
}

TEST(Nullable, Swap)
{
  Nullable<int> val1;
  Nullable<int> val2;
  Nullable<int> val3(12345);
  Nullable<int> val4(678910);

  val1.Swap(val2);
  EXPECT_FALSE(val1);
  EXPECT_FALSE(val2);

  val3.Swap(val4);
  EXPECT_TRUE(val3);
  EXPECT_TRUE(val4);
  EXPECT_TRUE(val3.Value() == 678910);
  EXPECT_TRUE(val4.Value() == 12345);

  val1.Swap(val3);
  EXPECT_TRUE(val1);
  EXPECT_FALSE(val3);
  EXPECT_TRUE(val1.Value() == 678910);
  EXPECT_FALSE(val3.HasValue());
}

TEST(Nullable, CopyConstruction)
{
  // Empty
  Nullable<int> val1;
  Nullable<int> val2(val1);
  EXPECT_FALSE(val1);
  EXPECT_FALSE(val2);

  // Non-Empty
  Nullable<int> val3(12345);
  Nullable<int> val4(val3);
  EXPECT_TRUE(val3);
  EXPECT_TRUE(val4);
  EXPECT_TRUE(val3.Value() == 12345);
  EXPECT_TRUE(val4.Value() == 12345);

  // Literal
  Nullable<int> val5 = 54321;
  EXPECT_TRUE(val5);
  EXPECT_TRUE(val5.Value() == 54321);

  // Value
  const int i = 1;
  Nullable<int> val6(i);
  EXPECT_TRUE(val6);
  EXPECT_TRUE(val6.Value() == 1);
}

TEST(Nullable, Disengage)
{
  Nullable<int> val1(12345);
  val1.Reset();
  EXPECT_FALSE(val1);
}

TEST(Nullable, ValueOr)
{
  Nullable<int> val1(12345);
  Nullable<int> val2;

  EXPECT_TRUE(val1);
  EXPECT_TRUE(val1.ValueOr(678910) == 12345);
  // Ensure the value was unmodified in ValueOr
  EXPECT_TRUE(val1.Value() == 12345);

  EXPECT_FALSE(val2);
  EXPECT_TRUE(val2.ValueOr(678910) == 678910);
  // Ensure val2 is still disengaged after call to ValueOr
  EXPECT_FALSE(val2);
}

void Foo(int&& rValue) { (void)rValue; }

#if GTEST_HAS_DEATH_TEST
TEST(Nullable, PreCondition)
{
  Nullable<int> emptyNullable;

#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(auto a = emptyNullable.Value(); (void)a;, "");
#else
  ASSERT_DEATH(auto a = emptyNullable.Value(); (void)a;, "Empty Nullable");
#endif
}

TEST(Nullable, PreCondition2)
{
  Nullable<int> emptyNullable;

#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(auto& a = emptyNullable.Value(); (void)a;, "");
#else
  ASSERT_DEATH(auto& a = emptyNullable.Value(); (void)a;, "Empty Nullable");
#endif
}

TEST(Nullable, PreCondition3)
{
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(Foo(Nullable<int>().Value());, "");
#else
  ASSERT_DEATH(Foo(Nullable<int>().Value());, "Empty Nullable");
#endif
}
#endif

TEST(Nullable, Operator)
{
  Nullable<std::string> val1("12345");
  EXPECT_EQ(*val1, "12345");
  val1->append("aaaa");
  EXPECT_EQ(*val1, "12345aaaa");
}

TEST(Nullable, Move)
{
  Nullable<std::unique_ptr<int>> val(std::make_unique<int>(123));
  std::unique_ptr<int> const taken = *std::move(val);
  EXPECT_TRUE(taken);
  EXPECT_EQ(*taken, 123);
  // val.HasValue() would return true, but accessing a value after it has been moved is UB anyways.
}

TEST(Nullable, ConstexprAndRvalue)
{
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpessimizing-move" // cspell:disable-line
#endif // __clang__

  Nullable<int> nullableInt0(std::move(Nullable<int>()));
  Nullable<int> nullableInt11(std::move(Nullable<int>(11)));

#if defined(__clang__)
#pragma clang diagnostic pop // NOLINT(clang-diagnostic-unknown-pragmas)
#endif // __clang__

  Nullable<int> nullableInt00(Nullable<int>{});
  Nullable<int> nullableInt1(Nullable<int>(1));

  EXPECT_FALSE(nullableInt0.HasValue());
  EXPECT_FALSE(nullableInt00.HasValue());

  nullableInt0.Reset();
  EXPECT_FALSE(nullableInt0.HasValue());

  EXPECT_TRUE(nullableInt1.HasValue());
  EXPECT_TRUE(nullableInt11.HasValue());

  EXPECT_EQ(*nullableInt1, 1);
  EXPECT_EQ(*nullableInt11, 11);

  std::string str(Nullable<std::string>(std::string("hello")).Value());
  EXPECT_EQ(str, "hello");
}

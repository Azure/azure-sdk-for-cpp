// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/nullable.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Nullable, Basic)
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
  EXPECT_TRUE(instance2.GetValue() == "hello world");

  auto instance3 = std::move(instance);
  EXPECT_TRUE(instance3.HasValue());
  EXPECT_TRUE(instance3.GetValue() == "hello world");

  EXPECT_TRUE(instance.HasValue());

  // This is not a guarantee that the string will be empty
  //  It is an implementation detail that the contents are moved
  //  Should a future compiler change this assumption this test will need updates
  EXPECT_TRUE(instance.GetValue() == "");
  EXPECT_TRUE(instance.HasValue());
}

TEST(Nullable, ValueAssignment)
{
  Nullable<int> intVal;
  EXPECT_FALSE(intVal.HasValue());
  intVal = 7;
  EXPECT_TRUE(intVal.HasValue());
  EXPECT_TRUE(intVal.GetValue() == 7);

  Nullable<double> doubleVal;
  EXPECT_FALSE(doubleVal.HasValue());
  doubleVal = 10.12345;
  EXPECT_TRUE(doubleVal.HasValue());
  EXPECT_TRUE(doubleVal.GetValue() == 10.12345);

  Nullable<std::string> strVal;
  EXPECT_FALSE(strVal.HasValue());
  strVal = std::string("Hello World");
  EXPECT_TRUE(strVal.HasValue());
  EXPECT_TRUE(strVal.GetValue() == "Hello World");

  strVal = "New String";
  EXPECT_TRUE(strVal.GetValue() == "New String");

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
  EXPECT_TRUE(val3.GetValue() == 678910);
  EXPECT_TRUE(val4.GetValue() == 12345);

  val1.Swap(val3);
  EXPECT_TRUE(val1);
  EXPECT_FALSE(val3);
  EXPECT_TRUE(val1.GetValue() == 678910);
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
  EXPECT_TRUE(val3.GetValue() == 12345);
  EXPECT_TRUE(val4.GetValue() == 12345);

  // Literal
  Nullable<int> val5 = 54321;
  EXPECT_TRUE(val5);
  EXPECT_TRUE(val5.GetValue() == 54321);

  // Value
  const int i = 1;
  Nullable<int> val6(i);
  EXPECT_TRUE(val6);
  EXPECT_TRUE(val6.GetValue() == 1);
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
  EXPECT_TRUE(val1.GetValue() == 12345);

  EXPECT_FALSE(val2);
  EXPECT_TRUE(val2.ValueOr(678910) == 678910);
  // Ensure val2 is still disengaged after call to ValueOr
  EXPECT_FALSE(val2);
}

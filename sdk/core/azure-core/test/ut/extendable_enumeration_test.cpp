//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/extendable_enumeration.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Azure::Core;

class MyEnum : public Azure::Core::_internal::ExtendableEnumeration<MyEnum> {
public:
  MyEnum(std::string initialValue) : ExtendableEnumeration(std::move(initialValue)) {}
  MyEnum() = default;

  static const MyEnum Value1;
  static const MyEnum Value2;
  static const MyEnum Value3;
};

const MyEnum MyEnum::Value1("Value1");
const MyEnum MyEnum::Value2("Value2");
const MyEnum MyEnum::Value3("Value3");

TEST(ExtendableEnumeration, BasicTests)
{
  {
    MyEnum enum1 = MyEnum::Value1;
    EXPECT_EQ(enum1, MyEnum::Value1);
  }
  {
    MyEnum enumToTest(MyEnum::Value2);
    EXPECT_NE(enumToTest, MyEnum::Value3);
  }

  {
    MyEnum enumVal;
    GTEST_LOG_(INFO) << enumVal.ToString();
  }

  {
    MyEnum enumVal(MyEnum::Value3);
    EXPECT_EQ(enumVal.ToString(), "Value3");
  }

  {
    MyEnum enumVal(MyEnum::Value1);
    MyEnum enumVal2(enumVal);
    EXPECT_EQ(enumVal, enumVal2);
  }
  {
    MyEnum enumVal(MyEnum::Value1);
    MyEnum enumVal2;
    EXPECT_EQ(enumVal, MyEnum::Value1);
    enumVal2 = enumVal;
    EXPECT_EQ(enumVal, enumVal2);
  }
}

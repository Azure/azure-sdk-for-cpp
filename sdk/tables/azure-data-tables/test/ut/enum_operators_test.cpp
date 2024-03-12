// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "enum_operators_test.hpp"

using namespace Azure::Data::Tables::Sas;
using namespace Azure::Data::Tables;
namespace Azure { namespace Data { namespace Test {
  TEST(EnumOperator, AndTest)
  {
    {
      auto val = TestEnum::One & TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::One & TestEnum::One;
      EXPECT_EQ(val, TestEnum::One);
    }
    {
      auto val = TestEnum::Two & TestEnum::One;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::Two & TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Two);
    }
    {
      auto val = TestEnum::One;
      val &= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::One;
      val &= TestEnum::One;
      EXPECT_EQ(val, TestEnum::One);
    }
    {
      auto val = TestEnum::Two;
      val &= TestEnum::One;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::Two;
      val &= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Two);
    }
  }

  TEST(EnumOperator, OrTest)
  {
    {
      auto val = TestEnum::One | TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::One | TestEnum::One;
      EXPECT_EQ(val, TestEnum::One);
    }
    {
      auto val = TestEnum::Two | TestEnum::One;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::Two | TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Two);
    }
    {
      auto val = TestEnum::One;
      val |= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::One;
      val |= TestEnum::One;
      EXPECT_EQ(val, TestEnum::One);
    }
    {
      auto val = TestEnum::Two;
      val |= TestEnum::One;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::Two;
      val |= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Two);
    }
  }

  TEST(EnumOperator, XorTest)
  {
    {
      auto val = TestEnum::One ^ TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::One ^ TestEnum::One;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::Two ^ TestEnum::One;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::Two ^ TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::One;
      val ^= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::One;
      val ^= TestEnum::One;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = TestEnum::Two;
      val ^= TestEnum::One;
      EXPECT_EQ(val, TestEnum::Three);
    }
    {
      auto val = TestEnum::Two;
      val ^= TestEnum::Two;
      EXPECT_EQ(val, TestEnum::Zero);
    }
  }
}}} // namespace Azure::Data::Test

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "enum_operators_test.hpp"

#include "azure/data/tables/enum_operators.hpp"

using namespace Azure::Data::Tables;

namespace Azure { namespace Data { namespace Test {
  TEST(EnumOperator, AndTest)
  {
    {
      constexpr auto val = TestEnum::One & TestEnum::Two;
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
      constexpr auto val = TestEnum::One | TestEnum::Two;
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
      constexpr auto val = TestEnum::One ^ TestEnum::Two;
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

  TEST(EnumOperator, ComplementTest)
  {
    {
      constexpr auto val = ~TestEnum::Zero;
      EXPECT_EQ(val, TestEnum::All);
    }
    {
      auto val = ~TestEnum::All;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = ~~TestEnum::Zero;
      EXPECT_EQ(val, TestEnum::Zero);
    }
    {
      auto val = ~~TestEnum::One;
      EXPECT_EQ(val, TestEnum::One);
    }
  }
}}} // namespace Azure::Data::Test

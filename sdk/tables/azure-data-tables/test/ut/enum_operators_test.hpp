#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// clang-format off
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {
  enum class TestEnum
  {
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
    All = ~0
  };

  class EnumOperator : public Azure::Core::Test::TestBase {};
}}} // namespace Azure::Data::Test

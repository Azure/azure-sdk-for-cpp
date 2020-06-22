// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure.hpp>
#include <string>

using namespace Azure::Core;

TEST(String, invariantCompare)
{
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("abc", "ABC"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("abc", "abcd"));
}

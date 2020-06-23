// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure.hpp>
#include <string>

using namespace Azure::Core;

TEST(String, invariantCompare)
{
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("", ""));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("a", "a"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("a", "A"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("aa", "AA"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("aA", "AA"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("abc", "ABC"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("", "a"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("a", ""));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("a", "aA"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("abc", "abcd"));
}

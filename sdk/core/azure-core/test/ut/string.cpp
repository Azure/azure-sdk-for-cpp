// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/strings.hpp"
#include "gtest/gtest.h"
#include <string>

TEST(String, invariantCompare)
{
  using Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual;
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("", ""));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("a", "a"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("A", "a"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("AA", "aa"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("aA", "aa"));
  EXPECT_TRUE(LocaleInvariantCaseInsensitiveEqual("ABC", "abc"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("", "a"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("a", ""));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("A", "aA"));
  EXPECT_FALSE(LocaleInvariantCaseInsensitiveEqual("ABC", "abcd"));
}

TEST(String, toLower)
{
  using Azure::Core::Strings::ToLower;
  EXPECT_TRUE(ToLower("") == "");
  EXPECT_TRUE(ToLower("a") == "a");
  EXPECT_TRUE(ToLower("A") == "a");
  EXPECT_TRUE(ToLower("AA") == "aa");
  EXPECT_TRUE(ToLower("aA") == "aa");
  EXPECT_TRUE(ToLower("ABC") == "abc");
  EXPECT_TRUE(ToLower("ABC-1-,!@#$%^&*()_+=ABC") == "abc-1-,!@#$%^&*()_+=abc");
  EXPECT_FALSE(ToLower("") == "a");
  EXPECT_FALSE(ToLower("a") == "");
  EXPECT_FALSE(ToLower("a") == "aA");
  EXPECT_FALSE(ToLower("abc") == "abcd");
}

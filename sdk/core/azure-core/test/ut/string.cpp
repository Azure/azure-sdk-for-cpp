// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/strings.hpp>
#include <gtest/gtest.h>
#include <string>

TEST(String, invariantCompare)
{
  using Azure::Core::_internal::StringExtensions;
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("", ""));
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("a", "a"));
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("A", "a"));
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("AA", "aa"));
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("aA", "aa"));
  EXPECT_TRUE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("ABC", "abc"));
  EXPECT_FALSE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("", "a"));
  EXPECT_FALSE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("a", ""));
  EXPECT_FALSE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("A", "aA"));
  EXPECT_FALSE(StringExtensions::LocaleInvariantCaseInsensitiveEqual("ABC", "abcd"));
}

TEST(String, toLower)
{
  using Azure::Core::_internal::StringExtensions;
  EXPECT_TRUE(StringExtensions::ToLower("") == "");
  EXPECT_TRUE(StringExtensions::ToLower("a") == "a");
  EXPECT_TRUE(StringExtensions::ToLower("A") == "a");
  EXPECT_TRUE(StringExtensions::ToLower("AA") == "aa");
  EXPECT_TRUE(StringExtensions::ToLower("aA") == "aa");
  EXPECT_TRUE(StringExtensions::ToLower("ABC") == "abc");
  EXPECT_TRUE(StringExtensions::ToLower("ABC-1-,!@#$%^&*()_+=ABC") == "abc-1-,!@#$%^&*()_+=abc");
  EXPECT_FALSE(StringExtensions::ToLower("") == "a");
  EXPECT_FALSE(StringExtensions::ToLower("a") == "");
  EXPECT_FALSE(StringExtensions::ToLower("a") == "aA");
  EXPECT_FALSE(StringExtensions::ToLower("abc") == "abcd");
}

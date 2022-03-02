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

TEST(String, toLowerC)
{
  using Azure::Core::_internal::StringExtensions;
  for (unsigned char ch = 0; ch < 255; ch += 1)
  {
    EXPECT_TRUE(StringExtensions::ToLower(ch) == std::tolower(ch));
  }
}

TEST(String, toUpperC)
{
  using Azure::Core::_internal::StringExtensions;
  for (unsigned char ch = 0; ch < 255; ch += 1)
  {
    EXPECT_TRUE(StringExtensions::ToUpper(ch) == std::toupper(ch));
  }
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
  EXPECT_TRUE(
      StringExtensions::ToLower("abcdefghijklmnopqrstuvwxyz") == "abcdefghijklmnopqrstuvwxyz");
  EXPECT_TRUE(
      StringExtensions::ToLower("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == "abcdefghijklmnopqrstuvwxyz");
  EXPECT_TRUE(StringExtensions::ToLower("ABC-1-,!@#$%^&*()_+=ABC") == "abc-1-,!@#$%^&*()_+=abc");
  EXPECT_FALSE(StringExtensions::ToLower("") == "a");
  EXPECT_FALSE(StringExtensions::ToLower("a") == "");
  EXPECT_FALSE(StringExtensions::ToLower("a") == "aA");
  EXPECT_FALSE(StringExtensions::ToLower("abc") == "abcd");
}

TEST(String, toUpper)
{
  using Azure::Core::_internal::StringExtensions;
  EXPECT_TRUE(StringExtensions::ToUpper("") == "");
  EXPECT_TRUE(StringExtensions::ToUpper("a") == "A");
  EXPECT_TRUE(StringExtensions::ToUpper("A") == "A");
  EXPECT_TRUE(StringExtensions::ToUpper("AA") == "AA");
  EXPECT_TRUE(StringExtensions::ToUpper("aA") == "AA");
  EXPECT_TRUE(
      StringExtensions::ToUpper("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_TRUE(StringExtensions::ToUpper("ABC") == "ABC");
  EXPECT_TRUE(
      StringExtensions::ToUpper("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_TRUE(StringExtensions::ToUpper("ABC-1-,!@#$%^&*()_+=ABC") == "ABC-1-,!@#$%^&*()_+=ABC");
  EXPECT_FALSE(StringExtensions::ToUpper("") == "A");
  EXPECT_FALSE(StringExtensions::ToUpper("a") == "");
  EXPECT_FALSE(StringExtensions::ToUpper("a") == "aA");
  EXPECT_FALSE(StringExtensions::ToUpper("abc") == "abcd");
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/strings.hpp>
#include <gtest/gtest.h>

#include <locale>
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
  for (unsigned i = 0; i <= 255; ++i)
  {
    auto const c = static_cast<char>(static_cast<unsigned char>(i));
    EXPECT_EQ(StringExtensions::ToLower(c), std::tolower(c, std::locale::classic()));
  }
}

TEST(String, toUpperC)
{
  using Azure::Core::_internal::StringExtensions;
  for (unsigned i = 0; i <= 255; ++i)
  {
    auto const c = static_cast<char>(static_cast<unsigned char>(i));
    EXPECT_EQ(StringExtensions::ToUpper(c), std::toupper(c, std::locale::classic()));
  }
}

TEST(String, toLower)
{
  using Azure::Core::_internal::StringExtensions;
  EXPECT_EQ(StringExtensions::ToLower(""), "");
  EXPECT_EQ(StringExtensions::ToLower("a"), "a");
  EXPECT_EQ(StringExtensions::ToLower("A"), "a");
  EXPECT_EQ(StringExtensions::ToLower("AA"), "aa");
  EXPECT_EQ(StringExtensions::ToLower("aA"), "aa");
  EXPECT_EQ(StringExtensions::ToLower("ABC"), "abc");
  EXPECT_EQ(StringExtensions::ToLower("abcdefghijklmnopqrstuvwxyz"), "abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(StringExtensions::ToLower("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), "abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(StringExtensions::ToLower("ABC-1-,!@#$%^&*()_+=ABC"), "abc-1-,!@#$%^&*()_+=abc");

  EXPECT_NE(StringExtensions::ToLower(""), "a");
  EXPECT_NE(StringExtensions::ToLower("a"), "");
  EXPECT_NE(StringExtensions::ToLower("a"), "aA");
  EXPECT_NE(StringExtensions::ToLower("abc"), "abcd");
}

TEST(String, toUpper)
{
  using Azure::Core::_internal::StringExtensions;
  EXPECT_EQ(StringExtensions::ToUpper(""), "");
  EXPECT_EQ(StringExtensions::ToUpper("a"), "A");
  EXPECT_EQ(StringExtensions::ToUpper("A"), "A");
  EXPECT_EQ(StringExtensions::ToUpper("AA"), "AA");
  EXPECT_EQ(StringExtensions::ToUpper("aA"), "AA");
  EXPECT_EQ(StringExtensions::ToUpper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_EQ(StringExtensions::ToUpper("ABC"), "ABC");
  EXPECT_EQ(StringExtensions::ToUpper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_EQ(StringExtensions::ToUpper("ABC-1-,!@#$%^&*()_+=ABC"), "ABC-1-,!@#$%^&*()_+=ABC");

  EXPECT_NE(StringExtensions::ToUpper(""), "A");
  EXPECT_NE(StringExtensions::ToUpper("a"), "");
  EXPECT_NE(StringExtensions::ToUpper("a"), "aA");
  EXPECT_NE(StringExtensions::ToUpper("abc"), "abcd");
}

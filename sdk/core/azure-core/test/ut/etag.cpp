// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/etag.hpp>

#include <limits>

using namespace Azure::Core;

TEST(ETag, Equality)
{
  auto et1 = ETag("tag");
  EXPECT_EQ(et1.ToString(), "tag");

  auto et2 = ETag("\"tag\"");
  EXPECT_EQ(et2.ToString(), "\"tag\"");

  auto et3 = ETag("W/\"weakETag\"");
  EXPECT_EQ(et3.ToString(), "W/\"weakETag\"");

  auto strongETag
      = ETag("\"#$%&'()*+,-./"
             "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");

  EXPECT_EQ(
      strongETag.ToString(),
      "\"#$%&'()*+,-./"
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");
}

TEST(ETag, IsWeak)
{
  auto et1 = ETag("tag");
  EXPECT_FALSE(et1.IsWeak());

  auto et2 = ETag("\"tag\"");
  EXPECT_FALSE(et2.IsWeak());

  auto et3 = ETag("W/\"weakETag\"");
  EXPECT_TRUE(et3.IsWeak());
  
  auto et4 = ETag("W/\"\"");
  EXPECT_TRUE(et4.IsWeak());
}

TEST(ETag, Equals)
{
  auto weakTag = ETag("W/\"\"");
  auto weakTag1 = ETag("W/\"1\"");
  auto weakTag2 = ETag("W/\"Two\"");
  auto strongTag1 = ETag("\"1\"");
  auto strongTag2 = ETag("\"Two\"");
  auto strongTagValidChars
      = ETag("\"#$%&'()*+,-./"
             "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");
  auto weakTagValidChars
      = ETag("W/\"#$%&'()*+,-./"
             "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");


  EXPECT_FALSE(weakTag == weakTag);
  EXPECT_FALSE(weakTag1 == weakTag1);
  EXPECT_FALSE(weakTag2 == weakTag2);
  EXPECT_FALSE(weakTagValidChars == weakTagValidChars);
  EXPECT_TRUE(strongTag1 == strongTag1);
  EXPECT_TRUE(strongTag2 == strongTag2);
  EXPECT_TRUE(strongTagValidChars == strongTagValidChars);

  EXPECT_TRUE(weakTag != weakTag);
  EXPECT_TRUE(weakTag1 != weakTag1);
  EXPECT_TRUE(weakTag2 != weakTag2);
  EXPECT_TRUE(weakTagValidChars != weakTagValidChars);
  EXPECT_FALSE(strongTag1 != strongTag1);
  EXPECT_FALSE(strongTag2 != strongTag2);
  EXPECT_FALSE(strongTagValidChars != strongTagValidChars);

  EXPECT_FALSE(weakTag == weakTag1);
  EXPECT_FALSE(weakTag1 == weakTag);
  EXPECT_FALSE(weakTagValidChars == strongTagValidChars);

  EXPECT_TRUE(weakTag != weakTag1);
  EXPECT_TRUE(weakTag1 != weakTag);
  EXPECT_TRUE(weakTagValidChars != strongTagValidChars);

  EXPECT_FALSE(weakTag1 == weakTag2);
  EXPECT_FALSE(weakTag1 == strongTag1);
  EXPECT_FALSE(strongTag1 == weakTag1);

  EXPECT_TRUE(weakTag1 != weakTag2);
  EXPECT_TRUE(weakTag1 != strongTag1);
  EXPECT_TRUE(strongTag1 != weakTag1);

  EXPECT_FALSE(weakTag2 == strongTag2);
  EXPECT_FALSE(strongTag2 == weakTag2);

  EXPECT_TRUE(weakTag2 != strongTag2);
  EXPECT_TRUE(strongTag2 != weakTag2);
}

TEST(ETag, Empty)
{
  auto anyETag = ETag::Any();
  auto nullETag = ETag::Null();
  auto strongETag
      = ETag("\"#$%&'()*+,-./"
             "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");

  EXPECT_TRUE(anyETag == ETag::Any());
  EXPECT_TRUE(nullETag == ETag::Null());

  EXPECT_FALSE(anyETag == nullETag);
  EXPECT_FALSE(anyETag == strongETag);
  EXPECT_FALSE(nullETag == strongETag);
}

TEST(ETag, CompareStrong)
{
  // W/""
  auto weakTag = ETag("W/\"\"");
  // W/"1"
  auto weakTag1 = ETag("W/\"1\"");
  // W/"Two"
  auto weakTagTwo = ETag("W/\"Two\"");
  // W/"two"
  auto weakTagtwo = ETag("W/\"two\"");
  // "1"
  auto strongTag1 = ETag("\"1\"");
  // "Two"
  auto strongTagTwo = ETag("\"Two\"");
  // "two"
  auto strongTagtwo = ETag("\"two\"");

  EXPECT_FALSE(ETag::Compare(weakTag, weakTag, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTag1, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTagTwo, weakTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTagtwo, weakTagtwo, ETag::ETagComparison::Strong));

  EXPECT_TRUE(ETag::Compare(strongTag1, strongTag1, ETag::ETagComparison::Strong));
  EXPECT_TRUE(ETag::Compare(strongTagTwo, strongTagTwo, ETag::ETagComparison::Strong));
  EXPECT_TRUE(ETag::Compare(strongTagtwo, strongTagtwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(weakTag, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTag1, weakTag, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(weakTag1, weakTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTagTwo, weakTag1, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(weakTag1, strongTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(strongTag1, weakTag1, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(weakTagTwo, strongTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(strongTagTwo, weakTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(strongTagTwo, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTag1, strongTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(strongTagTwo, strongTagtwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(strongTagtwo, strongTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Compare(weakTagTwo, weakTagtwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Compare(weakTagtwo, weakTagTwo, ETag::ETagComparison::Strong));
}

TEST(ETag, CompareWeak)
{
  // W/""
  auto weakTag = ETag("W/\"\"");
  // W/"1"
  auto weakTag1 = ETag("W/\"1\"");
  // W/"Two"
  auto weakTagTwo = ETag("W/\"Two\"");
  // W/"two"
  auto weakTagtwo = ETag("W/\"two\"");
  // "1"
  auto strongTag1 = ETag("\"1\"");
  // "Two"
  auto strongTagTwo = ETag("\"Two\"");
  // "two"
  auto strongTagtwo = ETag("\"two\"");

  EXPECT_TRUE(ETag::Compare(weakTag, weakTag, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(weakTag1, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(weakTagTwo, weakTagTwo, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(weakTagtwo, weakTagtwo, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Compare(strongTag1, strongTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(strongTagTwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(weakTag, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(weakTag1, weakTag, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(weakTag1, weakTagTwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(weakTagTwo, weakTag1, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Compare(weakTag1, strongTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(strongTag1, weakTag1, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Compare(weakTagTwo, strongTagTwo, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Compare(strongTagTwo, weakTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(strongTagTwo, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(weakTag1, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(strongTagTwo, weakTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(weakTagtwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(strongTagTwo, strongTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(strongTagtwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Compare(weakTagTwo, weakTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Compare(weakTagtwo, weakTagTwo, ETag::ETagComparison::Weak));
}

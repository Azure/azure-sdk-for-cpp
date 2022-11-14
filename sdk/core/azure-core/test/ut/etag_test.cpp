//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/etag.hpp>

#include <limits>

using namespace Azure;

TEST(ETag, ToString)
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
  {
    auto et1 = ETag("tag");
    EXPECT_FALSE(et1.IsWeak());

    auto et2 = ETag("\"tag\"");
    EXPECT_FALSE(et2.IsWeak());

    auto et3 = ETag("W/\"weakETag\"");
    EXPECT_TRUE(et3.IsWeak());

    auto et4 = ETag("W/\"\"");
    EXPECT_TRUE(et4.IsWeak());

    auto any = ETag::Any();
    EXPECT_FALSE(any.IsWeak());
  }

  {
    auto strong000 = ETag();
    auto strong00 = ETag("W/\"");
    auto strong0 = ETag("Xxxx"); // cspell:disable-line
    auto strong1 = ETag("Wxxx"); // cspell:disable-line
    auto strong2 = ETag("W/xx");
    auto strong3 = ETag("W/\"x");
    auto weak = ETag("W/\"/\"");

    ASSERT_FALSE(strong000.IsWeak());
    ASSERT_FALSE(strong00.IsWeak());
    ASSERT_FALSE(strong0.IsWeak());
    ASSERT_FALSE(strong1.IsWeak());
    ASSERT_FALSE(strong2.IsWeak());
    ASSERT_FALSE(strong3.IsWeak());
    ASSERT_TRUE(weak.IsWeak());
  }
}

TEST(ETag, Equals)
{
  ETag empty;
  ETag empty2;

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

  EXPECT_TRUE(empty == empty);
  EXPECT_TRUE(empty2 == empty2);
  EXPECT_TRUE(empty == empty2);

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

TEST(ETag, Any)
{
  auto nullETag = ETag();
  auto anyETag = ETag::Any();
  auto star = ETag("*");
  auto weakStar = ETag("W\"*\"");
  auto quotedStar = ETag("\"*\"");

  auto strongETag
      = ETag("\"#$%&'()*+,-./"
             "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\"");

  EXPECT_TRUE(anyETag == anyETag);
  EXPECT_TRUE(anyETag == ETag::Any());
  EXPECT_FALSE(anyETag == strongETag);

  EXPECT_TRUE(star == star);
  EXPECT_TRUE(star == ETag::Any());
  EXPECT_TRUE(star == anyETag);

  EXPECT_FALSE(star == weakStar);
  EXPECT_FALSE(weakStar == anyETag);
  EXPECT_FALSE(quotedStar == weakStar);

  EXPECT_FALSE(star == quotedStar);
  EXPECT_TRUE(anyETag == star);

  EXPECT_EQ(nullETag, nullETag);
  EXPECT_EQ(anyETag, anyETag);
  EXPECT_NE(nullETag, anyETag);
  EXPECT_NE(anyETag, nullETag);
}

TEST(ETag, EqualsStrong)
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

  EXPECT_FALSE(ETag::Equals(weakTag, weakTag, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTag1, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTagTwo, weakTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTagtwo, weakTagtwo, ETag::ETagComparison::Strong));

  EXPECT_TRUE(ETag::Equals(strongTag1, strongTag1, ETag::ETagComparison::Strong));
  EXPECT_TRUE(ETag::Equals(strongTagTwo, strongTagTwo, ETag::ETagComparison::Strong));
  EXPECT_TRUE(ETag::Equals(strongTagtwo, strongTagtwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(weakTag, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTag1, weakTag, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(weakTag1, weakTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTagTwo, weakTag1, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(weakTag1, strongTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(strongTag1, weakTag1, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(weakTagTwo, strongTagTwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(strongTagTwo, weakTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(strongTagTwo, weakTag1, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTag1, strongTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(strongTagTwo, strongTagtwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(strongTagtwo, strongTagTwo, ETag::ETagComparison::Strong));

  EXPECT_FALSE(ETag::Equals(weakTagTwo, weakTagtwo, ETag::ETagComparison::Strong));
  EXPECT_FALSE(ETag::Equals(weakTagtwo, weakTagTwo, ETag::ETagComparison::Strong));
}

TEST(ETag, EqualsWeak)
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

  EXPECT_TRUE(ETag::Equals(weakTag, weakTag, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(weakTag1, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(weakTagTwo, weakTagTwo, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(weakTagtwo, weakTagtwo, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Equals(strongTag1, strongTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(strongTagTwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(weakTag, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(weakTag1, weakTag, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(weakTag1, weakTagTwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(weakTagTwo, weakTag1, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Equals(weakTag1, strongTag1, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(strongTag1, weakTag1, ETag::ETagComparison::Weak));

  EXPECT_TRUE(ETag::Equals(weakTagTwo, strongTagTwo, ETag::ETagComparison::Weak));
  EXPECT_TRUE(ETag::Equals(strongTagTwo, weakTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(strongTagTwo, weakTag1, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(weakTag1, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(strongTagTwo, weakTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(weakTagtwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(strongTagTwo, strongTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(strongTagtwo, strongTagTwo, ETag::ETagComparison::Weak));

  EXPECT_FALSE(ETag::Equals(weakTagTwo, weakTagtwo, ETag::ETagComparison::Weak));
  EXPECT_FALSE(ETag::Equals(weakTagtwo, weakTagTwo, ETag::ETagComparison::Weak));
}

#if GTEST_HAS_DEATH_TEST
TEST(ETag, PreCondition)
{
  ETag emptyTag;

#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(emptyTag.ToString(), "");
#else
  ASSERT_DEATH(emptyTag.ToString(), "Empty ETag");
#endif
}
#endif

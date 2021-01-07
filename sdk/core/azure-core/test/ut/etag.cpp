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

}

TEST(ETag, IsWeak)
{
  auto et1 = ETag("tag");
  EXPECT_FALSE(et1.IsWeak());

  auto et2 = ETag("\"tag\"");
  EXPECT_FALSE(et2.IsWeak());

  auto et3 = ETag("W/\"weakETag\"");
  EXPECT_TRUE(et3.IsWeak());
}

TEST(ETag, StrongWeakComparison)
{
  // W/""
  auto weakTag = ETag("W/\"\"");
  EXPECT_TRUE(weakTag.IsWeak());

  // W/"1"
  auto weakTag1 = ETag("W/\"1\"");
  EXPECT_TRUE(weakTag1.IsWeak());

  // W/"2"
  auto weakTag2 = ETag("W/\"Two\"");
  EXPECT_TRUE(weakTag1.IsWeak());

  auto strongTag1 = ETag("\"1\"");
  EXPECT_FALSE(strongTag1.IsWeak());

  auto strongTag2 = ETag("\"Two\"");
  EXPECT_FALSE(strongTag2.IsWeak());

  EXPECT_TRUE(weakTag == weakTag);
  EXPECT_TRUE(weakTag1 == weakTag1);
  EXPECT_TRUE(weakTag2 == weakTag2);
  EXPECT_TRUE(strongTag1 == strongTag1);
  EXPECT_TRUE(strongTag2 == strongTag2);

  EXPECT_FALSE(weakTag == weakTag1);
  EXPECT_FALSE(weakTag1 == weakTag);
  EXPECT_TRUE(weakTag != weakTag1);
  EXPECT_TRUE(weakTag1 != weakTag);

  EXPECT_FALSE(weakTag1 == weakTag2);
  
  EXPECT_TRUE(weakTag1 == strongTag1);
  EXPECT_FALSE(strongTag1 == weakTag1);

  EXPECT_TRUE(weakTag2 == strongTag2);
  EXPECT_FALSE(strongTag2 == weakTag2);
}

TEST(ETag, Empty) { 
    auto anyETag = ETag::Any(); 
    auto nullETag = ETag::Null();
    auto strongETag = ETag("\"SomeInteresting1234567!@#$^&*ETAG___\"");

    EXPECT_TRUE(anyETag == ETag::Any());
    EXPECT_TRUE(nullETag == ETag::Null());

    EXPECT_FALSE(anyETag == nullETag);
    EXPECT_FALSE(anyETag == strongETag);
    EXPECT_FALSE(nullETag == strongETag);

}


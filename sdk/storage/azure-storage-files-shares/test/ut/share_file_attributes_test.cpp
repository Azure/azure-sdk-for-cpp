// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test/ut/test_base.hpp"

#include "azure/storage/files/shares/rest_client.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(ShareFileAttributes, EqualOperator)
  {
    Files::Shares::Models::FileAttributes a1 = Files::Shares::Models::FileAttributes::ReadOnly
        | Files::Shares::Models::FileAttributes::Hidden
        | Files::Shares::Models::FileAttributes::System;

    Files::Shares::Models::FileAttributes a2(a1.ToString());

    Files::Shares::Models::FileAttributes a3 =

        Files::Shares::Models::FileAttributes::System
        | Files::Shares::Models::FileAttributes::Offline
        | Files::Shares::Models::FileAttributes::ReadOnly
        | Files::Shares::Models::FileAttributes::Hidden;

    EXPECT_EQ(a1, a2);
    EXPECT_NE(a2, a3);
    EXPECT_TRUE(a1 == a2);
    EXPECT_FALSE(a2 == a3);
    EXPECT_FALSE(a1 != a2);
    EXPECT_TRUE(a2 != a3);
  }

  TEST(ShareFileAttributes, LogicOperator)
  {
    Files::Shares::Models::FileAttributes a1 = Files::Shares::Models::FileAttributes::ReadOnly
        | Files::Shares::Models::FileAttributes::Hidden;

    EXPECT_EQ(
        (a1 & Files::Shares::Models::FileAttributes::ReadOnly),
        Files::Shares::Models::FileAttributes::ReadOnly);
    EXPECT_NE(
        (a1 & Files::Shares::Models::FileAttributes::Offline),
        Files::Shares::Models::FileAttributes::Offline);

    Files::Shares::Models::FileAttributes a2;
    a2 |= Files::Shares::Models::FileAttributes::ReadOnly;
    a2 |= Files::Shares::Models::FileAttributes::Hidden;
    a2 |= Files::Shares::Models::FileAttributes::Hidden;
    a2 |= Files::Shares::Models::FileAttributes::Hidden;
    EXPECT_EQ(a1, a2);

    a2 &= Files::Shares::Models::FileAttributes::Offline;
    EXPECT_EQ(a2, Files::Shares::Models::FileAttributes());
    EXPECT_NE(a1, a2);

    a2 ^= Files::Shares::Models::FileAttributes::ReadOnly;
    a2 ^= Files::Shares::Models::FileAttributes::Hidden;
    EXPECT_EQ(a1, a2);
  }

  TEST(ShareFileAttributes, DefaultConstructible)
  {
    // default constructible
    Files::Shares::Models::FileAttributes a1;
    EXPECT_TRUE(a1.ToString().empty());
    EXPECT_TRUE(a1.GetValues().empty());

    Files::Shares::Models::FileAttributes a2("");
    EXPECT_TRUE(a2.ToString().empty());
    EXPECT_TRUE(a2.GetValues().empty());

    EXPECT_EQ(a1, a2);
  }

  TEST(ShareFileAttributes, RoundTrip)
  {
    Files::Shares::Models::FileAttributes a1 = Files::Shares::Models::FileAttributes::ReadOnly
        | Files::Shares::Models::FileAttributes::Hidden
        | Files::Shares::Models::FileAttributes::System
        | Files::Shares::Models::FileAttributes::None
        | Files::Shares::Models::FileAttributes::Directory
        | Files::Shares::Models::FileAttributes::Archive
        | Files::Shares::Models::FileAttributes::Temporary
        | Files::Shares::Models::FileAttributes::Offline
        | Files::Shares::Models::FileAttributes::NotContentIndexed
        | Files::Shares::Models::FileAttributes::NoScrubData;

    Files::Shares::Models::FileAttributes a2(a1.ToString());
    EXPECT_EQ(a1, a2);
  }

}}} // namespace Azure::Storage::Test
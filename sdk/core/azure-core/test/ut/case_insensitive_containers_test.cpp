// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/case_insensitive_containers.hpp>

#include <gtest/gtest.h>

using namespace Azure::Core;

TEST(CaseInsensitiveMap, Find)
{
  CaseInsensitiveMap imap;

  imap["Content-Length"] = "X";
  auto pos = imap.find("content-length");

  EXPECT_NE(pos, imap.end());
  EXPECT_EQ(pos->second, "X");
  EXPECT_EQ(pos->first, "Content-Length");
}

TEST(CaseInsensitiveMap, Modify)
{
  CaseInsensitiveMap imap;

  imap["Content-Length"] = "X";
  imap["content-length"] = "Y";
  auto pos = imap.find("CONTENT-LENGTH");

  EXPECT_NE(pos, imap.end());
  EXPECT_EQ(pos->second, "Y");
  EXPECT_EQ(pos->first, "Content-Length");
}

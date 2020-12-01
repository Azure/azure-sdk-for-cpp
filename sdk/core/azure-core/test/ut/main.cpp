// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <curl/curl.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);
  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();
  curl_global_cleanup();
  return r;
}

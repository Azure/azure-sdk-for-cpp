// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <curl/curl.h>
#endif

int main(int argc, char** argv)
{
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  curl_global_init(CURL_GLOBAL_ALL);
#endif

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  curl_global_cleanup();
#endif
  return r;
}

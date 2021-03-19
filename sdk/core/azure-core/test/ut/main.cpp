// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <curl/curl.h>
#include <signal.h>
#endif

int main(int argc, char** argv)
{
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  curl_global_init(CURL_GLOBAL_ALL);
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif
#endif

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  curl_global_cleanup();
#endif
  return r;
}

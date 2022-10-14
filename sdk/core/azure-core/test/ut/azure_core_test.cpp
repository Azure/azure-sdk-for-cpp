// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/platform.hpp>
#endif
#include <iostream>
#include <signal.h>

// C Runtime Library errors trigger SIGABRT. Catch that signal handler and report it on the test
// log.
void AbortHandler(int signal) { std::cout << "abort() has been called: " << signal << std::endl; }

int main(int argc, char** argv)
{
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif

  signal(SIGABRT, AbortHandler);

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}

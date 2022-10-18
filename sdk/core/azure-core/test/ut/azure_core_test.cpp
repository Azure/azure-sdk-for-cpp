// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/platform.hpp>
#endif
#include <csignal>
#include <iostream>

int main(int argc, char** argv)
{
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif

// Declare a signal handler to report unhandled exceptions on Windows - this is not needed for other
// OS's as they will print the exception to stderr in their terminate() function.
#if defined(AZ_PLATFORM_WINDOWS)
  // Ensure that all calls to abort() no longer pop up a modal dialog on Windows.
#if defined(_DEBUG) && defined(_MSC_VER)
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

  signal(SIGABRT, [](int) {
    // Rethrow any exceptions on the current stack - this will cause any pending exceptions to be
    // thrown so we can catch them and report them to the caller. This is needed because the
    // terminate() function on Windows calls abort() which normally pops up UI terminates without
    // reporting the exception.
    try
    {
      throw;
    }
    catch (std::exception const& ex)
    {
      std::cout << "Exception thrown: " << ex.what() << std::endl;
    }
  });
#endif // AZ_PLATFORM_WINDOWS

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}

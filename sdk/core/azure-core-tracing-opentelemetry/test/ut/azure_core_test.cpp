//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/internal/diagnostics/global_exception.hpp"
#include "azure/core/platform.hpp"

#include <csignal>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif
  // Declare a signal handler to report unhandled exceptions on Windows - this is not needed for
  // other
// OS's as they will print the exception to stderr in their terminate() function.
#if defined(AZ_PLATFORM_WINDOWS)
  // Ensure that all calls to abort() no longer pop up a modal dialog on Windows.
#if defined(_DEBUG) && defined(_MSC_VER)
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

  signal(SIGABRT, Azure::Core::Diagnostics::_internal::GlobalExceptionHandler::HandleSigAbort);
#endif // AZ_PLATFORM_WINDOWS

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/internal/diagnostics/global_exception.hpp"
#include "azure/core/platform.hpp"

#include <csignal>
#if ENABLE_RUST_AMQP
#include "rust_amqp_wrapper.h"
#endif
#include <gtest/gtest.h>

#if defined(AZ_PLATFORM_WINDOWS)
#if defined(_DEBUG) && defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
// MSVC CRT error callback.
// This function is called when a CRT error is detected.
// It will print the error message to the debugger output.
// On exit, the breakOnError flag indicates whether the CRT should break into the debugger on this
// error. The return value of this function determines if the CRT error processing logic should
// continue.
//
int ReportCrtError(int level, char* message, int* breakOnError)
{
  OutputDebugStringA(message);
  *breakOnError = FALSE;
  // If this is a CRT error, we want to break into the debugger, otherwise just print the
  // diagnostic.
  if (level == _CRT_ERROR && IsDebuggerPresent())
  {
    *breakOnError = TRUE;
  }
  return TRUE;
}
#endif // _DEBUG && _MSC_VER
#endif // AZ_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif

#if ENABLE_RUST_AMQP
  // Initialize the Rust AMQP logging.
  Azure::Core::Amqp::RustInterop::_detail::enable_tracing_integration();
#endif

  // Declare a signal handler to report unhandled exceptions on Windows - this is not needed for
  // other OS's as they will print the exception to stderr in their terminate() function.
#if defined(AZ_PLATFORM_WINDOWS)
  // Ensure that all calls to abort() no longer pop up a modal dialog on Windows.
#if defined(_DEBUG) && defined(_MSC_VER)
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
  // Enable CRT error reporting.
  _CrtSetReportHook(ReportCrtError);
#endif

  signal(SIGABRT, Azure::Core::Diagnostics::_internal::GlobalExceptionHandler::HandleSigAbort);
#endif // AZ_PLATFORM_WINDOWS

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file This test assumes the application is already using libcurl.
 *
 */

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/diagnostics/global_exception.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/response.hpp"
#include "http/curl/curl_connection_pool_private.hpp"
#include "http/curl/curl_connection_private.hpp"
#include "http/curl/curl_session_private.hpp"

#include <csignal>
#include <cstdlib>
#include <thread>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Test {
  // This test fails intermittently: https://github.com/Azure/azure-sdk-for-cpp/issues/4332
  TEST(SdkWithLibcurl, DISABLED_globalCleanUp)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://httpbin.org/get"));
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      auto connection = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                            .ExtractOrCreateCurlConnection(req, options);

      auto session
          = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
      session->Perform(Azure::Core::Context::ApplicationContext);
      // Reading all the response
      session->ReadToEnd(Azure::Core::Context::ApplicationContext);

      // If all three of these conditions are true, the connection should be moved to the connection
      // pool.
      EXPECT_TRUE(session->IsEOF());
      EXPECT_TRUE(session->m_keepAlive);
      EXPECT_FALSE(session->m_connectionUpgraded);
      t1 = high_resolution_clock::now();
    }
    // here the session is destroyed and the connection is moved to the pool
    // the same destructor also makes a call to start the cleanup thread
    // which will sleep for DefaultCleanerIntervalMilliseconds then loop through the connections
    // in the pool and check for the ones that are expired(DefaultConnectionExpiredMilliseconds) and
    // remove them which will be the case here if we wait long enough.
    // without the calculations below test is flaky due to the
    // fact that tests in the CI pipeline might take longer than 90 sec to execute thus the cleanup
    // thread strikes. to have this test be predictable we need to be aware of when we attempt to
    // read pool size, also we should let things run to completion and then check the pool size thus
    // the sleep below plus another second to let the for loop in the cleanup thread do its thing.

    auto t2 = high_resolution_clock::now();

    // Getting number of milliseconds as a double.
    duration<double, std::milli> ms_double = t2 - t1;
    if (ms_double < duration<double, std::milli>(
            Azure::Core::Http::_detail::DefaultCleanerIntervalMilliseconds))
    {
      // if the destructor execution took less than the cleanup thread sleep the size should be 1
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          1);
      // let the thread cleanup thread hit
      std::this_thread::sleep_for(
          std::chrono::milliseconds(
              Azure::Core::Http::_detail::DefaultCleanerIntervalMilliseconds + 1000)
          - ms_double);
      // Check that after the connection is gone and cleaned up, the pool is empty
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          0);
    }
    else
    {
      // we got back from the destructor and thread creation after the cleanup thread hit thus it
      // will be empty
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          0);
    }
  }
}}} // namespace Azure::Core::Test

int main(int argc, char** argv)
{
// Declare a signal handler to report unhandled exceptions on Windows - this is not needed for
// other OS's as they will print the exception to stderr in their terminate() function.
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

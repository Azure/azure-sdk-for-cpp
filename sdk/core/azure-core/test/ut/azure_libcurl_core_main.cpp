// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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

#include <gtest/gtest.h>

#include <curl/curl.h>

#include <azure/core/context.hpp>
#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/core/response.hpp>

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

#include <cstdlib>

namespace Azure { namespace Core { namespace Test {
  TEST(SdkWithLibcurl, globalCleanUp)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Http::Url("https://httpbin.org/get"));

    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      auto result = session->Perform(Azure::Core::Context::GetApplicationContext());
      (void)result;
      // Reading all the response

      Azure::Core::IO::BodyStream::ReadToEnd(
          *session, Azure::Core::Context::GetApplicationContext());
    }
    // Check that after the connection is gone, it is moved back to the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
    auto connectionFromPool
        = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin()->second.begin()->get();
    (void)connectionFromPool;
  }
}}} // namespace Azure::Core::Test

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  // When using libcurl, we need to call the transport adapter clean up routine.
  Azure::Core::Http::CurlTransport::CleanUp();

  // Call global clean up
  curl_global_cleanup();

  // return will destroy the connection pool, making each connection to call clean up.
  return r;
}

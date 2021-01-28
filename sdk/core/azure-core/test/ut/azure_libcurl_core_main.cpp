// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file This test assumes the application is already using libcurl.
 *
 */

#include <gtest/gtest.h>

#include <curl/curl.h>

#include <azure/core/context.hpp>
#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/response.hpp>

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

namespace Azure { namespace Core { namespace Test {
  TEST(SdkWithLibcurl, globalCleanUp)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Http::Url("http://httpbin.org/get"));

    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      auto result = session->Perform(Azure::Core::GetApplicationContext());
      // Reading all the response
      Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::GetApplicationContext(), *session);
    }
    // Check that after the connection is gone, it is moved back to the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
    auto connectionFromPool
        = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin()->second.begin()->get();
  }
}}} // namespace Azure::Core::Test

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  // Check that one connection is in the pool after running the test
  EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);

  // Call global clean up
  curl_global_cleanup();

  // return will destroy the connection pool, making each connection to call clean up.
  return r;
}

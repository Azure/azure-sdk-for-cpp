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

#include <azure/core/context.hpp>
#include <azure/core/http/curl_transport.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
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
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://httpbin.org/get"));

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
    }
    // Check that after the connection is gone, it is moved back to the pool
    EXPECT_EQ(
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
            .size(),
        1);
  }
}}} // namespace Azure::Core::Test

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();
  return r;
}

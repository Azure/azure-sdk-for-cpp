// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl/curl.hpp"
#endif

#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  // proxy server can take some minutes to handle the request. Only testing HTTP proxy
  // Test is disabled until there is a reliable proxy to be used for CI111
  TEST(CurlTransportOptions, DISABLED_proxy)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    // This proxy is currently alive but eventually we might want our own proxy server to be
    // available.
    curlOptions.Proxy = "136.228.165.138:8080";

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::TransportPolicyOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    Azure::Core::Http::Url url("http://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }

  /******************************* SSL options. ************************/
  TEST(CurlTransportOptions, noRevoke)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SSLOptions.EnableCertificateRevocationListCheck = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::TransportPolicyOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean uppon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear());
  }

  /*
  // Requires libcurl version >= 7.68
  TEST(CurlTransportOptions, nativeCA)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SSLOptions.NativeCa = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }

  // Requires libcurl version >= 7.70
  TEST(CurlTransportOptions, noPartialChain)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SSLOptions.NoPartialchain = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }

  // Requires libcurl version >= 7.71
  TEST(CurlTransportOptions, bestEffort)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SSLOptions.RevokeBestEffort = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }
  */

  TEST(CurlTransportOptions, sslVerifyOff)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    // If ssl verify is not disabled, this test would fail because the caInfo is not OK
    curlOptions.SSLVerifyPeer = false;
    // This ca info should be ignored by verify disable and test should work
    curlOptions.CAInfo = "/";

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::TransportPolicyOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    // Use https
    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean uppon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear());
  }

  TEST(CurlTransportOptions, httpsDefault)
  {
    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
    Azure::Core::Http::TransportPolicyOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

    // Use https
    Azure::Core::Http::Url url("https://httpbin.org/get");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean uppon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear());
  }

  TEST(CurlTransportOptions, disableKeepAlive)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.HttpKeepAlive = false;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::TransportPolicyOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy = std::make_unique<Azure::Core::Http::TransportPolicy>(options);

    {
      // use inner scope to remove the pipeline and make sure we don't keep the connection in the
      // pool
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      policies.emplace_back(std::move(transportPolicy));
      Azure::Core::Internal::Http::HttpPipeline pipeline(policies);

      Azure::Core::Http::Url url("http://httpbin.org/get");
      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

      std::unique_ptr<Azure::Core::Http::RawResponse> response;
      EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::GetApplicationContext(), request));
      auto responseCode = response->GetStatusCode();
      int expectedCode = 200;
      EXPECT_PRED2(
          [](int expected, int code) { return expected == code; },
          expectedCode,
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              responseCode));
    }
    // Make sure there are no connections in the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 0);
  }

}}} // namespace Azure::Core::Test

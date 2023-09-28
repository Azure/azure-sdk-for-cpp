// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>

#include <gtest/gtest.h>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#include "openssl/x509.h"
#endif

#include "transport_adapter_base_test.hpp"

#include <string>
#include <vector>

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

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
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
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
    curlOptions.SslOptions.EnableCertificateRevocationListCheck = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
    if (response)
    {
      auto responseCode = response->GetStatusCode();
      int expectedCode = 200;
      EXPECT_PRED2(
          [](int expected, int code) { return expected == code; },
          expectedCode,
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              responseCode));
    }

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean upon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                        .ConnectionPoolIndex.clear());
  }

  class CurlDerived : public Azure::Core::Http::CurlTransport {
    std::unique_ptr<Azure::Core::Http::CurlNetworkConnection> m_connection;

  public:
    void OnUpgradedConnection(
        std::unique_ptr<Azure::Core::Http::CurlNetworkConnection>&& connection) override
    {
      m_connection = std::move(connection);
    }
  };

  // Call the base OnUpgradedConnection method from a class derived from CurlTransport - this
  // primarily is there to increase code coverage.
  TEST(CurlTransportOptions, OnUpgradedConnection)
  {
    CurlDerived derived;
    std::unique_ptr<Azure::Core::Http::CurlNetworkConnection> connection;
    derived.OnUpgradedConnection(std::move(connection));
  }

  /*
  // Requires libcurl version >= 7.68
  TEST(CurlTransportOptions, nativeCA)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SslOptions.NativeCa = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy =
  std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::Context::ApplicationContext,
  request)); auto responseCode = response->GetStatusCode(); int expectedCode = 200; EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }

  // Requires libcurl version >= 7.70
  TEST(CurlTransportOptions, noPartialChain)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SslOptions.NoPartialchain = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy =
  std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::Context::ApplicationContext,
  request)); auto responseCode = response->GetStatusCode(); int expectedCode = 200; EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));
  }

  // Requires libcurl version >= 7.71
  TEST(CurlTransportOptions, bestEffort)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SslOptions.RevokeBestEffort = true;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    auto transportPolicy =
  std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(transportAdapter);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(Azure::Core::Context::ApplicationContext,
  request)); auto responseCode = response->GetStatusCode(); int expectedCode = 200; EXPECT_PRED2(
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
    curlOptions.SslVerifyPeer = false;
    // This ca info should be ignored by verify disable and test should work
    curlOptions.CAInfo = "/";

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    // Use HTTPS
    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean upon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                        .ConnectionPoolIndex.clear());
  }

#if defined(_azure_SUPPORT_SETTING_CAPATH)
  TEST(CurlTransportOptions, setCADirectory)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    // openssl default cert location will be used only if environment variable SSL_CERT_DIR
    // is not set
    const char* ca = getenv(X509_get_default_cert_dir_env());
    if (ca)
    {
      curlOptions.CAPath = ca;
    }
    else
    {
      curlOptions.CAPath = X509_get_default_cert_dir();
    }

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    // Use HTTPS
    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
    EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean upon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                        .ConnectionPoolIndex.clear());
  }
#endif

  TEST(CurlTransportOptions, httpsDefault)
  {
    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>();
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::move(transportPolicy));
    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    // Use HTTPS
    Azure::Core::Url url(AzureSdkHttpbinServer::Get());
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    std::unique_ptr<Azure::Core::Http::RawResponse> response;
    EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
    auto responseCode = response->GetStatusCode();
    int expectedCode = 200;
    EXPECT_PRED2(
        [](int expected, int code) { return expected == code; },
        expectedCode,
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            responseCode));

    // Clean the connection from the pool *Windows fails to clean if we leave to be clean upon
    // app-destruction
    EXPECT_NO_THROW(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                        .ConnectionPoolIndex.clear());
  }

  TEST(CurlTransportOptions, disableKeepAlive)
  {
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.HttpKeepAlive = false;

    auto transportAdapter = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    Azure::Core::Http::Policies::TransportOptions options;
    options.Transport = transportAdapter;
    auto transportPolicy
        = std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(options);

    {
      // use inner scope to remove the pipeline and make sure we don't keep the connection in the
      // pool
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
      policies.emplace_back(std::move(transportPolicy));
      Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

      Azure::Core::Url url(AzureSdkHttpbinServer::Get());
      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

      std::unique_ptr<Azure::Core::Http::RawResponse> response;
      EXPECT_NO_THROW(response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
      auto responseCode = response->GetStatusCode();
      int expectedCode = 200;
      EXPECT_PRED2(
          [](int expected, int code) { return expected == code; },
          expectedCode,
          static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
              responseCode));
    }
    // Make sure there are no connections in the pool
    EXPECT_EQ(
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
            .size(),
        0);
  }

}}} // namespace Azure::Core::Test

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief The base class for testing a curl session.
 *
 * @remark The curl connection mock is defined here.
 *
 */
#include "transport_adapter_base_test.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/response.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic push
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif // _MSC_VER

#include <azure/core/http/curl_transport.hpp>

#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <http/curl/curl_connection_pool_private.hpp>

namespace Azure { namespace Core { namespace Test {

  class CurlConnectionTest : public ::testing::Test {
  };

  TEST(CurlConnectionTest, ParseKeepAliveHeader)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));

    Azure::Core::Http::CurlConnection curlConnection(
        req, Azure::Core::Http::CurlTransportOptions(), "hostName", "propKey");
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=10");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("max=10, timeout=5");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5,max=10");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("max=10,timeout=5");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("max=10");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=10, extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=10, extra=1,");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=10, extra=1,  ");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5,  extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(5));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader(" max=10, extra=1,");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(10));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader(", , extra=1, ");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=,  extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout= ,  extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("max=,  extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("max= ,  extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=, max=10, extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=, extra=1,");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout= , max= , extra=1,  ");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout= , max=10, extra=1");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max= ,  extra=1,");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout= , max= , extra=1,  ");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5 max= ,  extra=1,");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout= , max= 10 extra=1,  ");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=x, max=10");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
    {
      auto parsedHeader = curlConnection.ParseKeepAliveHeader("timeout=5, max=n");
      EXPECT_EQ(parsedHeader.MaxRequests, size_t(0));
      EXPECT_EQ(parsedHeader.ConnectionTimeout, std::chrono::seconds(0));
    }
  }

  TEST(CurlConnectionTest, IsExpiredNot)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));
    req.SetHeader("Connection", "keep-alive");
    req.SetHeader("Keep-Alive", "timeout=120, max=2");
    Azure::Core::Http::CurlConnection curlConnection(
        req, Azure::Core::Http::CurlTransportOptions(), "hostName", "propKey");
    curlConnection.UpdateLastUsageTime();
    EXPECT_TRUE(!curlConnection.IsExpired());
  }

  TEST(CurlConnectionTest, IsExpiredMaxUsage)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));
    req.SetHeader("Connection", "keep-alive");
    req.SetHeader("Keep-Alive", "timeout=120, max=2");
    Azure::Core::Http::CurlConnection curlConnection(
        req, Azure::Core::Http::CurlTransportOptions(), "hostName", "propKey");
    curlConnection.IncreaseUsageCount();
    curlConnection.IncreaseUsageCount(); // usage == max
    curlConnection.UpdateLastUsageTime();
    EXPECT_TRUE(curlConnection.IsExpired());
    curlConnection.IncreaseUsageCount();
    curlConnection.IncreaseUsageCount(); // usage > max
    EXPECT_TRUE(curlConnection.IsExpired());
  }

  TEST(CurlConnectionTest, IsExpiredTimeout)
  {
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));
    req.SetHeader("Connection", "keep-alive");
    req.SetHeader("Keep-Alive", "timeout=1, max=2");
    Azure::Core::Http::CurlConnection curlConnection(
        req, Azure::Core::Http::CurlTransportOptions(), "hostName", "propKey");
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    curlConnection.UpdateLastUsageTime();
    EXPECT_TRUE(curlConnection.IsExpired());
  }
}}} // namespace Azure::Core::Test

#endif
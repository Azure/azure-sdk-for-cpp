// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/environment.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/platform.hpp"
#include "azure/core/response.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>

namespace Azure { namespace Core { namespace Test {
  namespace {
    constexpr static const char AzureSdkHttpbinServerSchema[] = "https";
    constexpr static const char AzureSdkHttpbinHost[] = "azuresdkforcpp.azurewebsites.net";
  } // namespace
  class TransportAdapterOptions : public ::testing::Test {

  public:
    enum class TestMode
    {
      UNKNOWN,
      RECORD,
      LIVE,
      PLAYBACK,
    };
    struct AzureSdkHttpbinServer final
    {
      inline static std::string Get()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + "/get";
      }
      inline static std::string Headers()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + "/headers";
      }
      inline static std::string WithPort()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + ":443/get";
      }
      inline static std::string Put()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + "/put";
      }
      inline static std::string Delete()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + "/delete";
      }
      inline static std::string Patch()
      {
        return std::string(AzureSdkHttpbinServerSchema) + "://" + std::string(AzureSdkHttpbinHost)
            + "/patch";
      }
      inline static std::string Host() { return std::string(AzureSdkHttpbinHost); }
      inline static std::string Schema() { return std::string(AzureSdkHttpbinServerSchema); }
    };

    static Azure::Core::Http::_internal::HttpPipeline CreateHttpPipeline(
        Azure::Core::Http::Policies::TransportOptions const& options);

    static void CheckBodyFromBuffer(
        Azure::Core::Http::RawResponse& response,
        int64_t size,
        std::string expectedBody = std::string(""));

    static void VerifyIsProxiedResponse(
        std::unique_ptr<Azure::Core::Http::RawResponse> const& response,
        std::string const& expectedOrigin);

    static std::string GetIpAddressFromHttpBinServer(
        std::unique_ptr<Azure::Core::Http::RawResponse> const& response);

    static void CheckBodyFromStream(
        Azure::Core::Http::RawResponse& response,
        int64_t size,
        std::string expectedBody = std::string(""));

    static void checkResponseCode(
        Azure::Core::Http::HttpStatusCode code,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok);

    std::string HttpProxyServer()
    {
#if defined(CODE_COVERAGE)
      GTEST_LOG_(INFO) << "Code Coverage server";
      return "http://127.0.0.1:3128";
#else
      std::string anonymousServer{
          Azure::Core::_internal::Environment::GetVariable("ANONYMOUSCONTAINERIPV4ADDRESS")};
      GTEST_LOG_(INFO) << "Anonymous server: " << anonymousServer;
      if (anonymousServer.empty())
      {
        GTEST_LOG_(ERROR)
            << "Could not find value for ANONYMOUSCONTAINERIPV4ADDRESS, Assuming local.";
        anonymousServer = "127.0.0.1";
      }

      return "http://" + anonymousServer + ":3128";
#endif
    }
    std::string HttpProxyServerWithPassword()
    {
#if defined(CODE_COVERAGE)
      GTEST_LOG_(INFO) << "Code Coverage server";
      return "http://127.0.0.1:3129";
#else
      std::string authenticatedServer{
          Azure::Core::_internal::Environment::GetVariable("AUTHENTICATEDCONTAINERIPV4ADDRESS")};
      GTEST_LOG_(INFO) << "Authenticated server: " << authenticatedServer;
      if (authenticatedServer.empty())
      {
        GTEST_LOG_(ERROR)
            << "Could not find value for AUTHENTICATEDCONTAINERIPV4ADDRESS, Assuming local.";
        authenticatedServer = "127.0.0.1";
      }
      return "http://" + authenticatedServer + ":3129";
#endif
    }

  protected:
    // Create
    virtual void SetUp() override {}

    TestMode GetTestMode()
    {
      auto value = Azure::Core::_internal::Environment::GetVariable("AZURE_TEST_MODE");
      GTEST_LOG_(INFO) << "Azure Test Mode: " << value;
      if (value.empty())
      {
        GTEST_LOG_(INFO) << "Assume Live Test";

        return TestMode::LIVE;
      }

      if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
              value, "RECORD"))
      {
        GTEST_LOG_(INFO) << "TestMode:: Record.";
        return TestMode::RECORD;
      }
      else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                   value, "PLAYBACK"))
      {
        GTEST_LOG_(INFO) << "TestMode:: Playback.";
        return TestMode::PLAYBACK;
      }
      else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                   value, "LIVE"))
      {
        GTEST_LOG_(INFO) << "TestMode:: Live.";
        return TestMode::LIVE;
      }

      // unexpected variable value
      throw std::runtime_error("Invalid environment variable: " + value);
    }
  };

  void TransportAdapterOptions::checkResponseCode(
      Azure::Core::Http::HttpStatusCode code,
      Azure::Core::Http::HttpStatusCode expectedCode)
  {
    EXPECT_PRED2(
        [](Azure::Core::Http::HttpStatusCode a, Azure::Core::Http::HttpStatusCode b) {
          return a == b;
        },
        code,
        expectedCode);
  }

  void TransportAdapterOptions::CheckBodyFromBuffer(
      Azure::Core::Http::RawResponse& response,
      int64_t size,
      std::string expectedBody)
  {
    auto body = response.ExtractBodyStream();
    EXPECT_EQ(body, nullptr);
    std::vector<uint8_t> bodyVector = response.GetBody();
    int64_t bodySize = bodyVector.size();

    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodySize, size);
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

  void TransportAdapterOptions::CheckBodyFromStream(
      Azure::Core::Http::RawResponse& response,
      int64_t size,
      std::string expectedBody)
  {
    auto body = response.ExtractBodyStream();
    EXPECT_NE(body, nullptr);

    std::vector<uint8_t> bodyVector = body->ReadToEnd(Azure::Core::Context::ApplicationContext);
    int64_t bodySize = body->Length();
    EXPECT_EQ(bodySize, size);
    bodySize = bodyVector.size();

    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodyVector.size(), static_cast<size_t>(size));
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

  std::string TransportAdapterOptions::GetIpAddressFromHttpBinServer(
      std::unique_ptr<Azure::Core::Http::RawResponse> const& response)
  {
    Azure::Core::Json::_internal::json jsonResponse
        = Azure::Core::Json::_internal::json::parse(response->GetBody());
    EXPECT_TRUE(jsonResponse.contains("origin"));
    EXPECT_TRUE(jsonResponse["origin"].is_string());
    return jsonResponse["origin"].get<std::string>();
  }
  void TransportAdapterOptions::VerifyIsProxiedResponse(
      std::unique_ptr<Azure::Core::Http::RawResponse> const& response,
      std::string const& expectedOrigin)
  {
    std::string ipaddress = GetIpAddressFromHttpBinServer(response);
    EXPECT_NE(expectedOrigin, ipaddress);
  }

  Azure::Core::Http::_internal::HttpPipeline TransportAdapterOptions::CreateHttpPipeline(
      Azure::Core::Http::Policies::TransportOptions const& transportOptions)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> pipelinePolicies;
    pipelinePolicies.push_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(
            transportOptions));

    return Azure::Core::Http::_internal::HttpPipeline{pipelinePolicies};
  }

  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Http::Policies::_internal;

#if defined(CODE_COVERAGE)
#else
#endif

  // constexpr char SocksProxyServer[] = "socks://98.162.96.41:4145";
  TEST_F(TransportAdapterOptions, SimpleProxyTests_LIVEONLY_)
  {
    // will skip test under some cased where test can't run (usually LIVE only tests)
#if !defined(CODE_COVERAGE)
    if (GetTestMode() != TestMode::LIVE)
    {
      GTEST_LOG_(INFO) << "Skipping live only test.";
      GTEST_SKIP();
    }
    else
    {
      GTEST_LOG_(INFO) << "Running live only test.";
    }
#endif
    Azure::Core::Url testUrl(AzureSdkHttpbinServer::Get());
    std::string myIpAddress;
    {
      // Construct a pipeline with a single transport policy not using a proxy.
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      Azure::Core::Http::_internal::HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
      myIpAddress = GetIpAddressFromHttpBinServer(response);
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServer();
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
      VerifyIsProxiedResponse(response, myIpAddress);
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServer();
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));
      testUrl.SetScheme("http");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
      VerifyIsProxiedResponse(response, myIpAddress);
    }
  }

#if !defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  typedef int CURLcode;
#endif

  TEST_F(TransportAdapterOptions, ProxyWithPasswordHttps_LIVENONLY_)
  {
    // will skip test under some cased where test can't run (usually LIVE only tests)
#if !defined(CODE_COVERAGE)
    if (GetTestMode() != TestMode::LIVE)
    {
      GTEST_SKIP();
    }
#endif
    Azure::Core::Url testUrl(AzureSdkHttpbinServer::Get());

    // HTTPS Connections.
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServerWithPassword();
      transportOptions.ProxyUserName = "user";
      transportOptions.ProxyPassword = "notthepassword";
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      try
      {
        // WinHTTP treats 407 authn errors from a proxy CONNECT as an HTTP server error, so deal
        // with it that way.
        auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
        EXPECT_EQ(
            response->GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::ProxyAuthenticationRequired);
      }
      catch (Azure::Core::Http::TransportException const&)
      {
        // CURL returns a connection error which triggers a transport exception.
        // See https://curl.se/mail/lib-2009-07/0078.html for more information.
      }
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServerWithPassword();
      transportOptions.ProxyUserName = "user";
      transportOptions.ProxyPassword = "password";
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapterOptions, ProxyWithPasswordHttp_LIVEONLY_)
  {
    // will skip test under some cased where test can't run (usually LIVE only tests)
#if !defined(CODE_COVERAGE)
    if (GetTestMode() != TestMode::LIVE)
    {
      GTEST_SKIP();
    }
#endif
    Azure::Core::Url testUrl(AzureSdkHttpbinServer::Get());
    // HTTP Connections.
    testUrl.SetScheme("http");
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServerWithPassword();
      transportOptions.ProxyUserName = "user";
      transportOptions.ProxyPassword = "notthepassword";
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      EXPECT_EQ(
          response->GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::ProxyAuthenticationRequired);
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServerWithPassword();
      transportOptions.ProxyUserName = "user";
      transportOptions.ProxyPassword = "password";
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }
}}} // namespace Azure::Core::Test
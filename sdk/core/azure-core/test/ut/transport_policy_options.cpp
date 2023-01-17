// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/policies/policy.hpp"
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif
#include "azure/core/internal/client_options.hpp"
#include "azure/core/internal/environment.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/platform.hpp"
#include "azure/core/response.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

namespace Azure { namespace Core { namespace Test {
  namespace {
    constexpr static const char AzureSdkHttpbinServerSchema[] = "https";
    constexpr static const char AzureSdkHttpbinHost[] = "azuresdkforcpp.azurewebsites.net";
  } // namespace
  class TransportAdapterOptions : public ::testing::Test {

  public:
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
      std::string proxyUrl{Azure::Core::_internal::Environment::GetVariable("SQUID_PROXY_URL")};
      if (proxyUrl.empty())
      {
        proxyUrl = "http://127.0.0.1:3128";
      }
      return proxyUrl;
    }
    std::string HttpProxyServerWithPassword()
    {
      std::string proxyUrl{
          Azure::Core::_internal::Environment::GetVariable("SQUID_AUTH_PROXY_URL")};
      if (proxyUrl.empty())
      {
        proxyUrl = "http://127.0.0.1:3129";
      }
      return proxyUrl;
    }
    std::string TestProxyUrl()
    {
      std::string proxyUrl{Azure::Core::_internal::Environment::GetVariable("PROXY_URL")};
      if (proxyUrl.empty())
      {
        proxyUrl = "https://localhost:5001";
      }
      return proxyUrl;
    }
    static bool ProxyStatusChecked;
    static bool IsSquidProxyRunning;
    static bool IsTestProxyRunning;

  protected:
    // Create
    virtual void SetUp() override
    {
#if defined(IN_CI_PIPELINE)
      // If we're in the CI pipeline, don't probe for the squid or test proxy running - just assume
      // they are.
      IsSquidProxyRunning = true;
      IsTestProxyRunning = true;
#else // !defined(IN_CI_PIPELINE)
      if (!ProxyStatusChecked)
      {
        Azure::Core::Http::Policies::TransportOptions options;
        {
          auto pipeline = CreateHttpPipeline(options);
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(HttpProxyServer()), false);
          try
          {
            auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
            IsSquidProxyRunning = true;
          }
          catch (Azure::Core::RequestFailedException& rfe)
          {
            IsSquidProxyRunning = false;
            std::cout << "Skipping proxy tests. Error: " << rfe.what() << std::endl;
          }
        }
        {
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
          Azure::Core::Http::CurlTransportOptions curlOptions;
          curlOptions.SslVerifyPeer = false;
          curlOptions.EnableCurlTracing = true;
          options.Transport = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
          Azure::Core::Http::WinHttpTransportOptions winHttpOptions;
          winHttpOptions.IgnoreUnknownCertificateAuthority = true;
          options.Transport = std::make_shared<Azure::Core::Http::WinHttpTransport>(winHttpOptions);
#endif
          auto pipeline = CreateHttpPipeline(options);
          auto request = Azure::Core::Http::Request(
              Azure::Core::Http::HttpMethod::Get,
              Azure::Core::Url(TestProxyUrl() + "/Admin/IsAlive"));
          try
          {
            pipeline.Send(request, Azure::Core::Context::ApplicationContext);
            IsTestProxyRunning = true;
          }
          catch (Azure::Core::RequestFailedException& rfe)
          {
            IsTestProxyRunning = false;
            std::cout << "Skipping TestProxy tests: " << rfe.what() << std::endl;
          }
        }
        ProxyStatusChecked = true;
      }
#endif
    }
  };
  bool TransportAdapterOptions::ProxyStatusChecked{false};
  bool TransportAdapterOptions::IsSquidProxyRunning{false};
  bool TransportAdapterOptions::IsTestProxyRunning{false};

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

#if !defined(DISABLE_PROXY_TESTS)
  // constexpr char SocksProxyServer[] = "socks://98.162.96.41:4145";
  TEST_F(TransportAdapterOptions, SimpleProxyTests)
  {
    if (!IsSquidProxyRunning)
    {
      GTEST_SKIP_("Skipping proxy tests because proxy is not running.");
    }
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
  }

#if !defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  typedef int CURLcode;
#endif

  TEST_F(TransportAdapterOptions, ProxyWithPasswordHttps)
  {
    if (!IsSquidProxyRunning)
    {
      GTEST_SKIP_("Skipping proxy tests because proxy is not running.");
    }
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

  TEST_F(TransportAdapterOptions, ProxyWithPasswordHttp)
  {
    if (!IsSquidProxyRunning)
    {
      GTEST_SKIP_("Skipping proxy tests because proxy is not running.");
    }
    Azure::Core::Url testUrl(AzureSdkHttpbinServer::Get());
    // HTTP Connections.
    testUrl.m_scheme = "http";
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

#endif // defined(DISABLE_PROXY_TESTS)

  TEST_F(TransportAdapterOptions, DisableCrlValidation)
  {
    Azure::Core::Url testUrl(AzureSdkHttpbinServer::Get());
    //    Azure::Core::Url testUrl("https://www.microsoft.com/");
    // HTTP Connections.
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = true;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    }
#if !defined(DISABLE_PROXY_TESTS)
    if (IsSquidProxyRunning)
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      transportOptions.HttpProxy = HttpProxyServerWithPassword();
      transportOptions.ProxyUserName = "user";
      transportOptions.ProxyPassword = "password";
      // Disable CA checks on proxy pipelines too.
      transportOptions.EnableCertificateRevocationListCheck = true;

      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
#endif
  }

  TEST_F(TransportAdapterOptions, CheckFailedCrlValidation)
  {
    // By default, for the Windows and Mac platforms, Curl uses
    // SCHANNEL/SECTRANSP for CRL validation. Those SSL protocols
    // don't have the same behaviors as OpenSSL does.
#if !defined(AZ_PLATFORM_WINDOWS) && !defined(AZ_PLATFORM_MAC)
    //    Azure::Core::Url
    //    testUrl("https://github.com/Azure/azure-sdk-for-cpp/blob/main/README.md");
    Azure::Core::Url testUrl("https://www.wikipedia.org");
    // For <reasons>, github URLs work just fine if CRL validation is off, but if enabled,
    // they fail. Let's use that fact to verify that CRL validation causes github
    // URLs to fail.
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = false;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
        auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
        EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
      }
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = true;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
        EXPECT_THROW(
            pipeline.Send(request, Azure::Core::Context::ApplicationContext),
            Azure::Core::Http::TransportException);
      }
    }
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      //
      // Retrieving the test URL should succeed if we allow failed CRL retrieval because
      // the certificate for the test URL doesn't contain a CRL distribution points extension,
      // and by default there is no platform CRL present.
      Azure::Core::Http::CurlTransportOptions curlOptions;
      curlOptions.EnableCurlTracing = true;
      curlOptions.SslOptions.AllowFailedCrlRetrieval = true;
      curlOptions.SslOptions.EnableCertificateRevocationListCheck = true;
      transportOptions.Transport = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);

      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, testUrl);
        auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
        EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
      }
    }
#endif
  }

  TEST_F(TransportAdapterOptions, MultipleCrlOperations)
  {
    // LetsEncrypt certificates don't contain a distribution point URL extension. While this seems
    // to work when run locally, it fails in the CI pipeline. "https://www.wikipedia.org" uses a
    // LetsEncrypt certificate, so when testing manually, it is important to add it to the list.
    std::vector<std::string> testUrls{
        AzureSdkHttpbinServer::Get(), // Uses a Microsoft/DigiCert certificate.
        "https://aws.amazon.com", // Uses a Amazon/Starfield Technologies certificate.
        "https://www.example.com/", // Uses a DigiCert certificate.
        "https://www.google.com/", // Uses a google certificate.
    };

    GTEST_LOG_(INFO) << "Basic test calls.";
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // FIrst verify connectivity to the test servers.
      transportOptions.EnableCertificateRevocationListCheck = false;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      for (auto const& target : testUrls)
      {
        GTEST_LOG_(INFO) << "Test " << target;
        Azure::Core::Url url(target);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        std::unique_ptr<Azure::Core::Http::RawResponse> response;
        EXPECT_NO_THROW(
            response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
        if (response && response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Found)
        {
          EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
        }
      }
    }

    // Now verify that once we enable CRL checks, we can still access the URLs.
    GTEST_LOG_(INFO) << "Test with CRL checks enabled";
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = true;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      for (auto const& target : testUrls)
      {
        GTEST_LOG_(INFO) << "Test " << target;
        Azure::Core::Url url(target);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        std::unique_ptr<Azure::Core::Http::RawResponse> response;
        EXPECT_NO_THROW(
            response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
        if (response && response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Found)
        {
          EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
        }
      }
    }

    // Now verify that once we enable CRL checks, we can still access the URLs.
    GTEST_LOG_(INFO) << "Test with CRL checks enabled. Iteration 2.";
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = true;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      for (auto const& target : testUrls)
      {
        GTEST_LOG_(INFO) << "Test " << target;
        Azure::Core::Url url(target);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        std::unique_ptr<Azure::Core::Http::RawResponse> response;
        EXPECT_NO_THROW(
            response = pipeline.Send(request, Azure::Core::Context::ApplicationContext));
        if (response && response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Found)
        {
          EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
        }
      }
    }
  }

  TEST_F(TransportAdapterOptions, TestRootCertificate)
  {
    // On Windows and OSX, setting a root certificate disables the default system certificate
    // store. That means that if we set the expected certificate, we won't be able to connect to
    // the server because the certificates root CA is not in the store.
#if defined(AZ_PLATFORM_LINUX)
    // cspell:disable
    std::string azurewebsitesCertificate
        = "MIIF8zCCBNugAwIBAgIQCq+mxcpjxFFB6jvh98dTFzANBgkqhkiG9w0BAQwFADBh"
          "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3"
          "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH"
          "MjAeFw0yMDA3MjkxMjMwMDBaFw0yNDA2MjcyMzU5NTlaMFkxCzAJBgNVBAYTAlVT"
          "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xKjAoBgNVBAMTIU1pY3Jv"
          "c29mdCBBenVyZSBUTFMgSXNzdWluZyBDQSAwMTCCAiIwDQYJKoZIhvcNAQEBBQAD"
          "ggIPADCCAgoCggIBAMedcDrkXufP7pxVm1FHLDNA9IjwHaMoaY8arqqZ4Gff4xyr"
          "RygnavXL7g12MPAx8Q6Dd9hfBzrfWxkF0Br2wIvlvkzW01naNVSkHp+OS3hL3W6n"
          "l/jYvZnVeJXjtsKYcXIf/6WtspcF5awlQ9LZJcjwaH7KoZuK+THpXCMtzD8XNVdm"
          "GW/JI0C/7U/E7evXn9XDio8SYkGSM63aLO5BtLCv092+1d4GGBSQYolRq+7Pd1kR"
          "EkWBPm0ywZ2Vb8GIS5DLrjelEkBnKCyy3B0yQud9dpVsiUeE7F5sY8Me96WVxQcb"
          "OyYdEY/j/9UpDlOG+vA+YgOvBhkKEjiqygVpP8EZoMMijephzg43b5Qi9r5UrvYo"
          "o19oR/8pf4HJNDPF0/FJwFVMW8PmCBLGstin3NE1+NeWTkGt0TzpHjgKyfaDP2tO"
          "4bCk1G7pP2kDFT7SYfc8xbgCkFQ2UCEXsaH/f5YmpLn4YPiNFCeeIida7xnfTvc4"
          "7IxyVccHHq1FzGygOqemrxEETKh8hvDR6eBdrBwmCHVgZrnAqnn93JtGyPLi6+cj"
          "WGVGtMZHwzVvX1HvSFG771sskcEjJxiQNQDQRWHEh3NxvNb7kFlAXnVdRkkvhjpR"
          "GchFhTAzqmwltdWhWDEyCMKC2x/mSZvZtlZGY+g37Y72qHzidwtyW7rBetZJAgMB"
          "AAGjggGtMIIBqTAdBgNVHQ4EFgQUDyBd16FXlduSzyvQx8J3BM5ygHYwHwYDVR0j"
          "BBgwFoAUTiJUIBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1Ud"
          "JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMHYG"
          "CCsGAQUFBwEBBGowaDAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQu"
          "Y29tMEAGCCsGAQUFBzAChjRodHRwOi8vY2FjZXJ0cy5kaWdpY2VydC5jb20vRGln"
          "aUNlcnRHbG9iYWxSb290RzIuY3J0MHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6Ly9j"
          "cmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcmwwN6A1oDOG"
          "MWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5j"
          "cmwwHQYDVR0gBBYwFDAIBgZngQwBAgEwCAYGZ4EMAQICMBAGCSsGAQQBgjcVAQQD"
          "AgEAMA0GCSqGSIb3DQEBDAUAA4IBAQAlFvNh7QgXVLAZSsNR2XRmIn9iS8OHFCBA"
          "WxKJoi8YYQafpMTkMqeuzoL3HWb1pYEipsDkhiMnrpfeYZEA7Lz7yqEEtfgHcEBs"
          "K9KcStQGGZRfmWU07hPXHnFz+5gTXqzCE2PBMlRgVUYJiA25mJPXfB00gDvGhtYa"
          "+mENwM9Bq1B9YYLyLjRtUz8cyGsdyTIG/bBM/Q9jcV8JGqMU/UjAdh1pFyTnnHEl"
          "Y59Npi7F87ZqYYJEHJM2LGD+le8VsHjgeWX2CJQko7klXvcizuZvUEDTjHaQcs2J"
          "+kPgfyMIOY1DMJ21NxOJ2xPRC/wAh/hzSBRVtoAnyuxtkZ4VjIOh";
    // cspell:enable

    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.ExpectedTlsRootCertificate = azurewebsitesCertificate;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      Azure::Core::Url url(AzureSdkHttpbinServer::Get());
      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
      auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
      EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    }
#endif
  }

  const std::string TestProxyHttpsCertificate =
      // cspell:disable
      "MIIDSDCCAjCgAwIBAgIUIoKu8Oao7j10TLNxaUG2Bs0FrRwwDQYJKoZIhvcNAQEL"
      "BQAwFDESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTIyMDgwNTIxMTcyM1oXDTIzMDgw"
      "NTIxMTcyM1owFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEF"
      "AAOCAQ8AMIIBCgKCAQEA0UPG7ER++5/9D/qa4SCtt7QvdHwcpidbwktPNU8iRW7V"
      "pIDPWS4goLp/+7+maT0Z/mqwSO3JDtm/dtdlr3F/5EMgyUExnYcvUixZAiyFyEwj"
      "j6wnAtNvqsg4rDqBlD17fuqTVsZm9Yo7QYub6p5PeznWYucOxRrczqFCiW4uj0Yk"
      "GgUHPPmCvhSDKowV8CYRHfkD6R8R4SFkoP3/uejXHxeXoYJNMWq5K0GqGaOZtNFB"
      "F7QWZHoLrRpZcY4h+DxwP3c+/FdlVcs9nstkF+EnTnwx5IRyKsaWb/pUEmYKvNDz"
      "wi6qnRUdu+DghZuvyZZDgwoYrSZokcbKumk0MsLC3QIDAQABo4GRMIGOMA8GA1Ud"
      "EwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGmMBYGA1UdJQEB/wQMMAoGCCsGAQUF"
      "BwMBMBcGA1UdEQEB/wQNMAuCCWxvY2FsaG9zdDA6BgorBgEEAYI3VAEBBCwMKkFT"
      "UC5ORVQgQ29yZSBIVFRQUyBkZXZlbG9wbWVudCBjZXJ0aWZpY2F0ZTANBgkqhkiG"
      "9w0BAQsFAAOCAQEARX4NxGbycdPVuqvu/CO+/LpWrEm1OcOl7N57/mD5npTIJT78"
      "TYtXk1J61akumKdf5CaBgCDRcl35LhioFZIMEsiOidffAp6t493xocncFBhIYYrZ"
      "HS6aKsZKPu8h3wOLpYu+zh7f0Hx6pkHPAfw4+knmQjDYomz/hTwuo/MuT8k6Ee7B"
      "NGWqxUamLI8bucuf2ZfT1XOq83uWaFF5KwAuVLhpzo39/TmPyYGnaoKRYf9QjabS"
      "LUjecMNLJFWHUSD4cKHvXJjDYZEiCiy+MdUDytWIsfw0fzAUjz9Qaz8YpZ+fXufM"
      "MNMNfyJHSMEMFIT2D1UaQiwryXWQWJ93OiSdjA==";

  const std::string InvalidTestProxyHttpsCertificate
      = "MIIIujCCBqKgAwIBAgITMwAxS6DhmVCLBf6MWwAAADFLoDANBgkqhkiG9w0BAQwF"
        "ADBZMQswCQYDVQQGEwJVUzEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0aW9u"
        "MSowKAYDVQQDEyFNaWNyb3NvZnQgQXp1cmUgVExTIElzc3VpbmcgQ0EgMDEwHhcN"
        "MjIwMzE0MTgzOTU1WhcNMjMwMzA5MTgzOTU1WjBqMQswCQYDVQQGEwJVUzELMAkG"
        "A1UECBMCV0ExEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBD"
        "b3Jwb3JhdGlvbjEcMBoGA1UEAwwTKi5henVyZXdlYnNpdGVzLm5ldDCCASIwDQYJ"
        "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAM3heDMqn7v8cmh4A9vECuEfuiUnKBIw"
        "7y0Sf499Z7WW92HDkIvV3eJ6jcyq41f2UJcG8ivCu30eMnYyyI+aRHIedkvOBA2i"
        "PqG78e99qGTuKCj9lrJGVfeTBJ1VIlPvfuHFv/3JaKIBpRtuqxCdlgsGAJQmvHEn"
        "vIHUV2jgj4iWNBDoC83ShtWg6qV2ol7yiaClB20Af5byo36jVdMN6vS+/othn3jG"
        "pn+NP00DWYbP5y4qhs5XLH9wQZaTUPKIaUxmHewErcM0rMAaWl8wMqQTeNYf3l5D"
        "ax50yuEg9VVjtbDdSmvOkslGpVqsOl1NrmyN7gCvcvcRUQcxIiXJQc0CAwEAAaOC"
        "BGgwggRkMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdgCt9776fP8QyIudPZwe"
        "PhhqtGcpXc+xDCTKhYY069yCigAAAX+Jw/reAAAEAwBHMEUCIE8AAjvwO4AffPn7"
        "un67WykJ2hGB4n8qJE7pk4QYjWW+AiEA/pio1E9ALt30Kh/Ga4gRefH1ILbQ8n4h"
        "bHFatezIcvYAdwB6MoxU2LcttiDqOOBSHumEFnAyE4VNO9IrwTpXo1LrUgAAAX+J"
        "w/qlAAAEAwBIMEYCIQCdbj6FOX6wK+dLoqjWKuCgkKSsZsJKpVik6HjlRgomzQIh"
        "AM7mYp5dBFmNLas3fFcP0rMMK+17n8u0GhFH2KpkPr1SAHYA6D7Q2j71BjUy51co"
        "vIlryQPTy9ERa+zraeF3fW0GvW4AAAF/icP6jgAABAMARzBFAiAhjTz3PBjqRrpY"
        "eH7us44lESC7c0dzdTcehTeAwmEyrgIhAOCaqmqA+ercv+39jzFWkctG36bazRFX"
        "4gGNiKU0bctcMCcGCSsGAQQBgjcVCgQaMBgwCgYIKwYBBQUHAwIwCgYIKwYBBQUH"
        "AwEwPAYJKwYBBAGCNxUHBC8wLQYlKwYBBAGCNxUIh73XG4Hn60aCgZ0ujtAMh/Da"
        "HV2ChOVpgvOnPgIBZAIBJTCBrgYIKwYBBQUHAQEEgaEwgZ4wbQYIKwYBBQUHMAKG"
        "YWh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY2VydHMvTWljcm9zb2Z0"
        "JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMSUyMC0lMjB4c2lnbi5j"
        "cnQwLQYIKwYBBQUHMAGGIWh0dHA6Ly9vbmVvY3NwLm1pY3Jvc29mdC5jb20vb2Nz"
        "cDAdBgNVHQ4EFgQUiiks5RXI6IIQccflfDtgAHndN7owDgYDVR0PAQH/BAQDAgSw"
        "MHwGA1UdEQR1MHOCEyouYXp1cmV3ZWJzaXRlcy5uZXSCFyouc2NtLmF6dXJld2Vi"
        "c2l0ZXMubmV0ghIqLmF6dXJlLW1vYmlsZS5uZXSCFiouc2NtLmF6dXJlLW1vYmls"
        "ZS5uZXSCFyouc3NvLmF6dXJld2Vic2l0ZXMubmV0MAwGA1UdEwEB/wQCMAAwZAYD"
        "VR0fBF0wWzBZoFegVYZTaHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9j"
        "cmwvTWljcm9zb2Z0JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMS5j"
        "cmwwZgYDVR0gBF8wXTBRBgwrBgEEAYI3TIN9AQEwQTA/BggrBgEFBQcCARYzaHR0"
        "cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9Eb2NzL1JlcG9zaXRvcnkuaHRt"
        "MAgGBmeBDAECAjAfBgNVHSMEGDAWgBQPIF3XoVeV25LPK9DHwncEznKAdjAdBgNV"
        "HSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwDQYJKoZIhvcNAQEMBQADggIBAKtk"
        "4nEDfqxbP80uaoBoPaeeX4G/tBNcfpR2sf6soW8atAqGOohdLPcE0n5/KJn+H4u7"
        "CsZdTJyUVxBxAlpqAc9JABl4urWNbhv4pueGBZXOn5K5Lpup/gp1HhCx4XKFno/7"
        "T22NVDol4LRLUTeTkrpNyYLU5QYBQpqlFMAcvem/2seiPPYghFtLr5VWVEikUvnf"
        "wSlECNk84PT7mOdbrX7T3CbG9WEZVmSYxMCS4pwcW3caXoSzUzZ0H1sJndCJW8La"
        "9tekRKkMVkN558S+FFwaY1yARNqCFeK+yiwvkkkojqHbgwFJgCFWYy37kFR9uPiv"
        "3sTHvs8IZ5K8TY7rHk3pSMYqoBTODCs7wKGiByWSDMcfAgGBzjt95SKfq0p6sj0C"
        "+HWFiyKR+PTi2esFP9Vr9sC9jfRM6zwa7KnONqLefHauJPdNMt5l1FQGWvyco4IN"
        "lwK3Z9FfEOFZA4YcjsqnkNacKZqLjgis3FvD8VPXETgRuffVc75lJxH6WmkwqdXj"
        "BlU8wOcJyXTmM1ehYpziCpWvGBSEIsFuK6BC/iBnQEuWKdctAdbHIDlLctGgDWjx"
        "xYDPZ/TtORGL8YaDnj6QHeOURIAHCtt6NCWKV6OR2HtMx+tCEvfi5ION1dyJ9hAX"
        "+4K9FXc71ab7tdV/GLPkWc8Q0x1nk7ogDYcqKbiF";

  class TestProxy {
    // cspell:enable

    std::unique_ptr<HttpPipeline> m_pipeline;
    std::string TestProxyUrl()
    {
      std::string proxyUrl{Azure::Core::_internal::Environment::GetVariable("PROXY_URL")};
      if (proxyUrl.empty())
      {
        proxyUrl = "https://localhost:5001";
      }
      return proxyUrl;
    }

  public:
    struct TestProxyOptions : Azure::Core::_internal::ClientOptions
    {
      TestProxyOptions() : Azure::Core::_internal::ClientOptions() {}
    };
    TestProxy(TestProxyOptions options = TestProxyOptions())
    {
      if (options.Transport.ExpectedTlsRootCertificate.empty())
      {
        options.Transport.ExpectedTlsRootCertificate = TestProxyHttpsCertificate;
      }
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies;
      m_pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          "Test Proxy",
          "2021-11",
          std::move(perRetryPolicies),
          std::move(perCallPolicies));
    }

    Azure::Response<std::string> PostStartRecording(std::string const& recordingFile)
    {
      std::string proxyServerRequest;
      proxyServerRequest = "{ \"x-recording-file\": \"";
      proxyServerRequest += Azure::Core::Url::Encode(recordingFile);
      proxyServerRequest += "\"}";
      std::vector<uint8_t> bodyVector{proxyServerRequest.begin(), proxyServerRequest.end()};
      Azure::Core::IO::MemoryBodyStream postBody(bodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Post,
          Azure::Core::Url(TestProxyUrl() + "/record/start"),
          &postBody);

      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      auto& responseHeaders = response->GetHeaders();
      auto responseId = responseHeaders.find("x-recording-id");
      return Azure::Response<std::string>(responseId->second, std::move(response));
    }
    Azure::Response<Azure::Core::Http::HttpStatusCode> PostStopRecording(
        std::string const& recordingId)
    {
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Post, Azure::Core::Url(TestProxyUrl() + "/record/stop"));
      request.SetHeader("x-recording-id", recordingId);

      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      auto responseCode = response->GetStatusCode();
      return Azure::Response<Azure::Core::Http::HttpStatusCode>(responseCode, std::move(response));
    }

    Azure::Response<std::string> PostStartPlayback(std::string const& recordingFile)
    {
      std::string proxyServerRequest;
      proxyServerRequest = "{ \"x-recording-file\": \"";
      proxyServerRequest += Azure::Core::Url::Encode(recordingFile);
      proxyServerRequest += "\"}";
      std::vector<uint8_t> bodyVector{proxyServerRequest.begin(), proxyServerRequest.end()};
      Azure::Core::IO::MemoryBodyStream postBody(bodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Post,
          Azure::Core::Url(TestProxyUrl() + "/playback/start"),
          &postBody);

      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      auto& responseHeaders = response->GetHeaders();
      auto responseId = responseHeaders.find("x-recording-id");
      return Azure::Response<std::string>(responseId->second, std::move(response));
    }

    Azure::Response<Azure::Core::Http::HttpStatusCode> PostStopPlayback(
        std::string const& recordingId)
    {
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Post, Azure::Core::Url(TestProxyUrl() + "/playback/stop"));
      request.SetHeader("x-recording-id", recordingId);

      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      auto responseCode = response->GetStatusCode();
      return Azure::Response<Azure::Core::Http::HttpStatusCode>(responseCode, std::move(response));
    }

    Azure::Response<std::string> ProxyServerGetUrl(
        std::string const& recordingId,
        bool isRecording,
        std::string const& urlToRecord)
    {
      Azure::Core::Url targetUrl{urlToRecord};
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Get,
          Azure::Core::Url(TestProxyUrl() + "/" + targetUrl.GetRelativeUrl()));
      request.SetHeader(
          "x-recording-upstream-base-uri", targetUrl.GetScheme() + "://" + targetUrl.GetHost());
      request.SetHeader("x-recording-id", recordingId);
      request.SetHeader("x-recording-mode", (isRecording ? "record" : "playback"));

      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      std::string responseBody(response->GetBody().begin(), response->GetBody().end());
      return Azure::Response<std::string>(responseBody, std::move(response));
    }

    Azure::Response<Azure::Core::Http::HttpStatusCode> IsAlive()
    {
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(TestProxyUrl() + "/Admin/IsAlive"));
      auto response = m_pipeline->Send(request, Azure::Core::Context::ApplicationContext);
      auto statusCode = response->GetStatusCode();
      return Azure::Response<Azure::Core::Http::HttpStatusCode>(statusCode, std::move(response));
    }

    ~TestProxy() {}
  };

  TEST_F(TransportAdapterOptions, AccessTestProxyServer)
  {
    if (!IsTestProxyRunning)
    {
      GTEST_SKIP_("Skipping TestProxy tests because TestProxy is not running.");
    }

    TestProxy proxyServer;

    EXPECT_EQ(Azure::Core::Http::HttpStatusCode::Ok, proxyServer.IsAlive().Value);

    std::string recordingId;
    EXPECT_NO_THROW(recordingId = proxyServer.PostStartRecording("testRecording.json").Value);

    GTEST_LOG_(INFO) << "Started recording with ID " << recordingId;

    std::string response;
    EXPECT_NO_THROW(
        response
        = proxyServer.ProxyServerGetUrl(recordingId, true, AzureSdkHttpbinServer::Get()).Value);

    GTEST_LOG_(INFO) << "Response for recording " << recordingId << "is: " << response;

    EXPECT_NO_THROW(proxyServer.PostStopRecording(recordingId));

    EXPECT_NO_THROW(recordingId = proxyServer.PostStartPlayback("testRecording.json").Value);
    GTEST_LOG_(INFO) << "Started playback with ID " << recordingId;

    EXPECT_NO_THROW(
        response
        = proxyServer.ProxyServerGetUrl(recordingId, false, AzureSdkHttpbinServer::Get()).Value);

    GTEST_LOG_(INFO) << "Recorded Response for " << recordingId << "is: " << response;

    EXPECT_NO_THROW(proxyServer.PostStopPlayback(recordingId));
  }

  TEST_F(TransportAdapterOptions, TestProxyServerWithInvalidCertificate)
  {
    if (!IsTestProxyRunning)
    {
      GTEST_SKIP_("Skipping TestProxy tests because TestProxy is not running.");
    }

    TestProxy::TestProxyOptions options;
    options.Transport.ExpectedTlsRootCertificate = InvalidTestProxyHttpsCertificate;
    TestProxy proxyServer(options);

    EXPECT_THROW(proxyServer.IsAlive(), Azure::Core::Http::TransportException);
  }

}}} // namespace Azure::Core::Test

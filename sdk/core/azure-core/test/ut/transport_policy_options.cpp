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

using namespace std::chrono_literals;

#if !defined(DISABLE_PROXY_TESTS)
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

    std::string HttpProxyServer() { return "http://127.0.0.1:3128"; }
    std::string HttpProxyServerWithPassword() { return "http://127.0.0.1:3129"; }

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

  // constexpr char SocksProxyServer[] = "socks://98.162.96.41:4145";
  TEST_F(TransportAdapterOptions, SimpleProxyTests)
  {
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

  TEST_F(TransportAdapterOptions, ProxyWithPasswordHttps)
  {
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

  TEST_F(TransportAdapterOptions, DisableCaValidation)
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
    std::vector<std::string> testUrls{
        AzureSdkHttpbinServer::Get(),
        "https://www.microsoft.com/",
        "https://www.example.com/",
        "https://www.google.com/",
    };
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // FIrst verify connectivity to the test servers.
      transportOptions.EnableCertificateRevocationListCheck = false;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      for (auto const& target : testUrls)
      {
        Azure::Core::Url url(target);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
        if (response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Found)
        {
          EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
        }
      }
    }

    // Now verify that once we enable CRL checks, we can still access the URLs.
    {
      Azure::Core::Http::Policies::TransportOptions transportOptions;

      // Note that the default is to *disable* CRL checks, because they are disabled
      // by default. So we test *enabling* CRL validation checks.
      transportOptions.EnableCertificateRevocationListCheck = true;
      HttpPipeline pipeline(CreateHttpPipeline(transportOptions));

      for (auto const& target : testUrls)
      {
        Azure::Core::Url url(target);
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
        auto response = pipeline.Send(request, Azure::Core::Context::ApplicationContext);
        if (response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Found)
        {
          EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
        }
      }
    }
  }

  TEST_F(TransportAdapterOptions, TestRootCertificate)
  {
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
  }

}}} // namespace Azure::Core::Test
#endif // defined(DISABLE_PROXY_TESTS)

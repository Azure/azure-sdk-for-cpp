// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the HTTP send performance.
 *
 */

#pragma once

#include "../../../core/perf/inc/azure/perf.hpp"

#include <azure/core.hpp>
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/http/curl_transport.hpp>
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include <azure/core/http/win_http_transport.hpp>
#endif

#include <memory>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Measure the HTTP transport performance.
   */
  class HTTPTransportTest : public Azure::Perf::PerfTest {

    std::string m_target;
    std::shared_ptr<Azure::Core::Http::HttpTransport> m_transport;
    Azure::Core::Http::HttpMethod m_httpMethod = Azure::Core::Http::HttpMethod::Get;

    void GetRequest() const
    {
      auto httpRequest = Azure::Core::Http::Request(m_httpMethod, Azure::Core::Url(m_target));
      Azure::Core::Context context;
      auto response = m_transport->Send(httpRequest, context);
      // Make sure to pull all bytes from network.
      auto body = response->ExtractBodyStream()->ReadToEnd();
    }

    void PostRequest() const
    {
      std::string payload = "{}";
      Azure::Core::IO::MemoryBodyStream payloadStream(
          reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
      auto httpRequest
          = Azure::Core::Http::Request(m_httpMethod, Azure::Core::Url(m_target), &payloadStream);
      Azure::Core::Context context;
      auto response = m_transport->Send(httpRequest, context);
      // Make sure to pull all bytes from network.
      auto body = response->ExtractBodyStream()->ReadToEnd();
    }

  public:
    /**
     * @brief Construct a new HTTPTransportTest test.
     *
     * @param options The test options.
     */
    HTTPTransportTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    void Setup() override
    {
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
      if ("winhttp" == m_options.GetMandatoryOption<std::string>("Transport"))
      {

        Azure::Core::Http::WinHttpTransportOptions transportOptions;
        transportOptions.IgnoreInvalidCertificateCommonName = true;
        transportOptions.IgnoreUnknownCertificateAuthority = true;
        m_transport = std::make_shared<Azure::Core::Http::WinHttpTransport>(transportOptions);
      }
#endif
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
      if ("curl" == m_options.GetMandatoryOption<std::string>("Transport"))
      {

        Azure::Core::Http::CurlTransportOptions transportOptions;
        transportOptions.SslVerifyPeer = false;
        m_transport = std::make_shared<Azure::Core::Http::CurlTransport>(transportOptions);

      }
#endif
      m_httpMethod
          = Azure::Core::Http::HttpMethod(m_options.GetMandatoryOption<std::string>("Method"));

      if (m_httpMethod == Azure::Core::Http::HttpMethod::Get)
      {
        m_target = GetTestProxy() + "/Admin/isAlive";
      }
      else if (m_httpMethod == Azure::Core::Http::HttpMethod::Post)
      {
        m_target = GetTestProxy() + "/Admin/setRecordingOptions";
      }
    }
    
    /**
     * @brief Use HTTPTransportTest to call test proxy endpoint.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      try
      {
        if (m_httpMethod == Azure::Core::Http::HttpMethod::Get)
        {
          GetRequest();
        }
        else if (m_httpMethod == Azure::Core::Http::HttpMethod::Post)
        {
          PostRequest();
        }
      }
      catch (std::exception const&)
      {
        // don't print exceptions, they are happening at each request, this is the point of the test
      }
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"Method", {"--method"}, "The HTTP method e.g. GET, POST etc.", 1, true},
          {"Transport", {"--transport"}, "The HTTP Transport curl/winhttp.", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "HTTPTransportTest",
          "Measures HTTP transport performance",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::HTTPTransportTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test

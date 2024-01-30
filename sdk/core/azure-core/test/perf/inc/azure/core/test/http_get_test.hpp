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
#include <azure/core/http/curl_transport.hpp>
#include <azure/core/http/win_http_transport.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Measure the Curl performance.
   */
  class HTTPGetTest : public Azure::Perf::PerfTest {

    std::string m_target;
    std::shared_ptr<Azure::Core::Http::HttpTransport> m_Transport;

  public:
    /**
     * @brief Construct a new HTTPGetTest test.
     *
     * @param options The test options.
     */
    HTTPGetTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    void Setup() override
    {
      m_target = GetTestProxy() + "/Admin/isAlive";
      if ("winhttp" == m_options.GetMandatoryOption<std::string>("Transport"))
      {
        Azure::Core::Http::WinHttpTransportOptions transportOptions;
        transportOptions.IgnoreInvalidCertificateCommonName = true;
        transportOptions.IgnoreUnknownCertificateAuthority = true;
        m_Transport = std::make_shared<Azure::Core::Http::WinHttpTransport>(transportOptions);
      }
      else
      {
        Azure::Core::Http::CurlTransportOptions transportOptions;
        transportOptions.SslVerifyPeer = false;
        m_Transport = std::make_shared<Azure::Core::Http::CurlTransport>(transportOptions);
      }
    }

    /**
     * @brief Use HTTPCurlTest to call test proxy endpoint.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      try
      {
        Azure::Core::Context context;
        auto request = Azure::Core::Http::Request(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(m_target));
        auto response = m_Transport->Send(request, context);
        // Make sure to pull all bytes from network.
        auto body = response->ExtractBodyStream()->ReadToEnd();
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
      return {{"Transport", {"--transport"}, "The HTTP Transport curl/winhttp.", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "HTTPGetTest", "Measures HTTP Get performance", [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::HTTPGetTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test

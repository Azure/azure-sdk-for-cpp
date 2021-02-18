// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <azure/core/http/body_stream.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/transport.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace PerformanceStress { namespace Test {

  namespace Details {
    static std::unique_ptr<Azure::Core::Http::HttpTransport> HttpClient;
  } // namespace Details

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class HttpClientGetTest : public Azure::PerformanceStress::PerformanceTest {
  protected:
    Azure::Core::Http::Url m_url;

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    HttpClientGetTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief Get and set the url option
     *
     */
    void Setup() override
    {
      m_url = Azure::Core::Http::Url(m_options.GetMandatoryOption<std::string>("url"));
    }

    /**
     * @brief The test definition
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const& ctx) override
    {
      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, m_url);
      auto response = Details::HttpClient->Send(ctx, request);
      // Read the body from network
      auto bodyStream = response->GetBodyStream();
      response->SetBody(Azure::Core::Http::BodyStream::ReadToEnd(ctx, *bodyStream));
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return {{"url", {"--url"}, "Url to send the http request. *Required parameter.", 1, true}};
    }
  };

}}} // namespace Azure::PerformanceStress::Test

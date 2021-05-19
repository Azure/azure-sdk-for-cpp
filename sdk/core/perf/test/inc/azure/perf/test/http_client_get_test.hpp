// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/core/http/http.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/io/body_stream.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace Perf { namespace Test {

  namespace _detail {
    static std::unique_ptr<Azure::Core::Http::HttpTransport> HttpClient;
  } // namespace _detail

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class HttpClientGetTest : public Azure::Perf::PerfTest {
  protected:
    Azure::Core::Url m_url;

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    HttpClientGetTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Get and set the URL option
     *
     */
    void Setup() override
    {
      m_url = Azure::Core::Url(m_options.GetMandatoryOption<std::string>("url"));
    }

    /**
     * @brief The test definition
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const& ctx) override
    {
      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, m_url);
      auto response = _detail::HttpClient->Send(request, ctx);
      // Read the body from network
      auto bodyStream = response->ExtractBodyStream();
      response->SetBody(bodyStream->ReadToEnd(ctx));
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {{"url", {"--url"}, "Url to send the HTTP request. *Required parameter.", 1, true}};
    }
  };

}}} // namespace Azure::Perf::Test

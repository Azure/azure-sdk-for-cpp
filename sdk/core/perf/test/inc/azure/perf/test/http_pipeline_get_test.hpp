// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Performance test measuring the use of an HTTP pipeline (and optionally test proxy).
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <azure/core/http/http.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace Perf { namespace Test {

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class HttpPipelineGetTest : public Azure::Perf::PerfTest {
    Azure::Core::Url m_url;

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    HttpPipelineGetTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Get and set the URL option
     *
     */
    void Setup() override
    {
      m_url = Azure::Core::Url(m_options.GetMandatoryOption<std::string>("url"));
    }

    /**
     * @brief Set up the HTTP client
     *
     */
    void GlobalSetup() override {}

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "httpPipelineGet",
          "Send an HTTP GET request to a configurable URL using Azure Pipelines.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Perf::Test::HttpPipelineGetTest>(options);
          }};
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

    /**
     * @brief The test definition
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const& ctx) override
    {
      Azure::Core::_internal::ClientOptions clientOptions;

      ConfigureClientOptions(clientOptions);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRequest;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetry;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOptions, "PipelineTest", "na", std::move(perRequest), std::move(perRetry));

      Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, m_url);
      auto response = pipeline.Send(request, ctx);
      response->GetBody();
    }
  };

}}} // namespace Azure::Perf::Test

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include "azure/perf/test/http_client_get_test.hpp"

#include <azure/core/http/curl_transport.hpp>

namespace Azure { namespace Perf { namespace Test {
  /**
   * @brief A performance test that defines a test option.
   *
   */
  class CurlHttpClientGetTest : public Azure::Perf::Test::HttpClientGetTest {
  private:
    Azure::Core::Url m_url;

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    CurlHttpClientGetTest(Azure::Perf::TestOptions options) : HttpClientGetTest(options) {}

    /**
     * @brief Set up the HTTP client
     *
     */
    void GlobalSetup() override
    {
      m_httpClient = std::make_unique<Azure::Core::Http::CurlTransport>();
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "curlHttpClientGet",
          "Send an HTTP GET request to a configurable URL using libcurl.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Perf::Test::CurlHttpClientGetTest>(options);
          }};
    }
  };

}}} // namespace Azure::Perf::Test
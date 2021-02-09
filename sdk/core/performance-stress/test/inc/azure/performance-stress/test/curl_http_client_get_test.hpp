// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include "azure/performance-stress/test/http_client_get_test.hpp"

#include <azure/core/http/curl/curl.hpp>

#include <curl/curl.h>

namespace Azure { namespace PerformanceStress { namespace Test {
  /**
   * @brief A performance test that defines a test option.
   *
   */
  class CurlHttpClientGetTest : public Azure::PerformanceStress::Test::HttpClientGetTest {
  private:
    Azure::Core::Http::Url m_url;

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    CurlHttpClientGetTest(Azure::PerformanceStress::TestOptions options)
        : HttpClientGetTest(options)
    {
    }

    /**
     * @brief Set up the http client
     *
     */
    void GlobalSetup() override
    {
      curl_global_init(CURL_GLOBAL_ALL);
      Details::HttpClient = std::make_unique<Azure::Core::Http::CurlTransport>();
    }

    void GlobalCleanup() override { curl_global_cleanup(); }
  };

}}} // namespace Azure::PerformanceStress::Test

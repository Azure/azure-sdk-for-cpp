// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include "azure/performance-stress/test/http_client_get_test.hpp"

#include <azure/core/http/winhttp/win_http_client.hpp>

namespace Azure { namespace PerformanceStress { namespace Test {

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class WinHttpClientGetTest : public Azure::PerformanceStress::Test::HttpClientGetTest {
  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    WinHttpClientGetTest(Azure::PerformanceStress::TestOptions options) : HttpClientGetTest(options)
    {
    }

    /**
     * @brief Set up the http client
     *
     */
    void GlobalSetup() override
    {
      Details::HttpClient = std::make_unique<Azure::Core::Http::WinHttpTransport>();
    }
  };

}}} // namespace Azure::PerformanceStress::Test

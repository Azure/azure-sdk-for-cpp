//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief HTTP client that plays back NetworkCallRecord NetworkCallRecords.
 */

#pragma once

#include <string>

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/core/response.hpp>

#include "azure/core/test/network_models.hpp"

namespace Azure { namespace Core { namespace Test {

  // Partial class. Required to reference the Interceptor that is defined in the implementation.
  class InterceptorManager;

  /**
   * @brief Creates an HTTP Transport adapter that answer to requests using recorded data.
   *
   */
  class PlaybackClient : public Azure::Core::Http::HttpTransport {
  private:
    Azure::Core::Test::InterceptorManager* m_interceptorManager;

  public:
    /**
     * @brief Construct a new Playback Client that uses \p recordedData to answer to the HTTP
     * request.
     *
     * @param interceptorManager A reference to the interceptor manager holding the recorded data.
     */
    PlaybackClient(Azure::Core::Test::InterceptorManager* interceptorManager)
        : m_interceptorManager(interceptorManager)
    {
    }

    /**
     * @brief Override the HTTPTransport `send` contract.
     *
     * @param context The context that can cancel the request.
     * @param request The HTTP request details.
     * @return The HTTP raw response containing code, headers and payload.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) override;
  };

}}} // namespace Azure::Core::Test

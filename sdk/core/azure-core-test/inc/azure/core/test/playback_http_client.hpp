// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief HTTP client that plays back NetworkCallRecord NetworkCallRecords.
 */

#pragma once

#include <string>

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/response.hpp>

#include "azure/core/test/network_models.hpp"


namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Creates an HTTP Transport adapter that answer to requests using recorded data.
   *
   */
  class PlaybackClient : public Azure::Core::Http::HttpTransport {
  private:
    Azure::Core::Test::RecordedData& m_recordedData;

  public:
    /**
     * @brief Construct a new Playback Client that uses \p recordedData to answer to the HTTP
     * request.
     *
     * @param recordedData
     */
    PlaybackClient(Azure::Core::Test::RecordedData& recordedData) : m_recordedData(recordedData) {}

    /**
     * @brief Override the HTTPTransport `send` contract.
     *
     * @param context The context that can cancel the request.
     * @param request The HTTP request details.
     * @return The HTTP raw response containing code, headers and payload.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) override;
  };

}}} // namespace Azure::Core::Test

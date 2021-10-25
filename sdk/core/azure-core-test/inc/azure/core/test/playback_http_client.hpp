// Copyright (c) Microsoft Corporation. All rights reserved.
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
   * @brief A body stream which holds the memory inside.
   *
   * @remark The playback http uses this body stream to be returned as part of the raw response so
   * the transport policy can read from it.
   *
   */
  class WithMemoryBodyStream : public Azure::Core::IO::BodyStream {
  private:
    std::vector<uint8_t> m_memory;
    Azure::Core::IO::MemoryBodyStream m_streamer;

    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
    {
      return m_streamer.Read(buffer, count, context);
    }

  public:
    // Forbid constructor for rval so we don't end up storing dangling ptr
    WithMemoryBodyStream(std::vector<uint8_t> const&&) = delete;

    /**
     * @brief Construct using vector of bytes.
     *
     * @param buffer Vector of bytes with the contents to provide the data from to the readers.
     */
    WithMemoryBodyStream(std::vector<uint8_t> const& buffer)
        : m_memory(buffer), m_streamer(m_memory)
    {
    }

    int64_t Length() const override { return m_streamer.Length(); }

    void Rewind() override { m_streamer.Rewind(); }
  };

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

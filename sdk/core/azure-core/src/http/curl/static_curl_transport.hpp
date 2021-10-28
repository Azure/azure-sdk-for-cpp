// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport static implementation using libcurl won't produce real
 * response streams. The HTTP responses are first statically downloaded within the transport
 * adapter.
 *
 * @remark This transport adapater is less efficient than the non-static version. Use this
 * implementation where performance ( memory and time ) is not a concern.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"

namespace Azure { namespace Core { namespace Http {

  /**
   * @brief Concrete implementation of an HTTP Transport that uses libcurl.
   */
  class StaticCurlTransport final : public HttpTransport {
  private:
    CurlTransportOptions m_options;

  public:
    /**
     * @brief Construct a new CurlTransport object.
     *
     * @param options Optional parameter to override the default options.
     */
    StaticCurlTransport(CurlTransportOptions const& options = CurlTransportOptions())
        : m_options(options)
    {
    }

    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse
     *
     * @param request an HTTP Request to be send.
     * @param context A context to control the request lifetime.
     *
     * @return unique ptr to an HTTP RawResponse.
     */
    std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;
  };

}}} // namespace Azure::Core::Http

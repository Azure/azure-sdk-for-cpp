// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #HttpTransport implementation via CURL.
 */

#pragma once

#ifdef BUILD_CURL_HTTP_TRANSPORT_ADAPTER

#include "azure/core/context.hpp"
#include "azure/core/http/curl/curl connection_pool.hpp"
#include "azure/core/http/curl/curl_connection.hpp"
#include "azure/core/http/curl/curl_session.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"

namespace Azure { namespace Core { namespace Http {

  /**
   * @brief Concrete implementation of an HTTP Transport that uses libcurl.
   *
   */
  class CurlTransport : public HttpTransport {
  public:
    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse
     *
     * @param context #Context so that operation can be canceled.
     * @param request an HTTP Request to be send.
     * @return unique ptr to an HTTP RawResponse.
     */
    std::unique_ptr<RawResponse> Send(Context const& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http

#endif

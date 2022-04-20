// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport implementations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"

namespace Azure { namespace Core { namespace Http {

  namespace _internal {
    /**
     * @brief A service client can use this options to set up a transport adapter for some specific
     * needs.
     *
     */
    struct HttpServiceTransportOptions final
    {
      /**
       * @brief Ignores if a service requests a client certificate for a TLS connection.
       *
       * @note Only for WinHttp client.
       *
       */
      bool IgnoreClientCertificateAuthenticationOnWinHttp = false;
    };
  } // namespace _internal

  /**
   * @brief Base class for all HTTP transport implementations.
   */
  class HttpTransport {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions

    /**
     * @brief Send an HTTP request over the wire.
     *
     * @param request An #Azure::Core::Http::Request to send.
     * @param context A context to control the request lifetime.
     */
    // TODO - Should this be const
    virtual std::unique_ptr<RawResponse> Send(Request& request, Context const& context) = 0;

    /**
     * @brief Destructs `%HttpTransport`.
     *
     */
    virtual ~HttpTransport() {}

  protected:
    /**
     * @brief Constructs a default instance of `%HttpTransport`.
     *
     */
    HttpTransport() = default;

    /**
     * @brief Constructs `%HttpTransport` by copying another instance of `%HttpTransport`.
     *
     * @param other An instance to copy.
     */
    HttpTransport(const HttpTransport& other) = default;

    /**
     * @brief Constructs `%HttpTransport` by moving another instance of `%HttpTransport`.
     *
     * @param other An instance to move in.
     */
    HttpTransport(HttpTransport&& other) = default;

    /**
     * @brief Assigns `%HttpTransport` to another instance of `%HttpTransport`.
     *
     * @param other An instance to assign.
     *
     * @return A reference to this instance.
     */
    HttpTransport& operator=(const HttpTransport& other) = default;
  };

}}} // namespace Azure::Core::Http

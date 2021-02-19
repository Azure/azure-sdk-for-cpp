// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via CURL.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"

namespace Azure { namespace Core { namespace Http {

  /**
   * @brief The available options to set libcurl ssl options.
   *
   * @remark The SDK will map the enum option to libcurl's specific option. See more info here:
   * https://curl.haxx.se/libcurl/c/CURLOPT_SSL_OPTIONS.html
   *
   */
  struct CurlTransportSSLOptions
  {
    /**
     * @brief This option can enable the revocation list check.
     *
     * @remark Libcurl does revocation list check by default for ssl backends that supports this
     * feature. However, the Azure SDK overrides libcurl's behavior and disables the revocation list
     * check by default.
     *
     */
    bool EnableCertificateRevocationListCheck = false;
  };

  /**
   * @brief Set the curl connection options like a proxy and CA path.
   *
   */
  struct CurlTransportOptions
  {
    /**
     * @brief The string for the proxy is passed directly to the libcurl handle without any parsing
     *
     * @remark No validation for the string is done by the Azure SDK. More about this option:
     * https://curl.haxx.se/libcurl/c/CURLOPT_PROXY.html.
     *
     * @remark The default value is an empty string (no proxy).
     *
     */
    std::string Proxy;
    /**
     * @brief The string for the certificate authenticator is sent to libcurl handle directly.
     *
     * @remark The Azure SDK will not check if the path is valid or not.
     *
     * @remark The default is the built-in system specific path. More about this option:
     * https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html
     *
     */
    std::string CAInfo;
    /**
     * @brief All HTTP requests will keep the connection channel open to the service.
     *
     * @remark The channel might be closed by the server if the server response has an error code.
     * A connection won't be re-used if it is abandoned in the middle of an operation.
     * operation.
     *
     * @remark This option is managed directly by the Azure SDK. No option is set for the curl
     * handle. It is `true` by default.
     */
    bool HttpKeepAlive = true;
    /**
     * @brief This option determines whether curl verifies the authenticity of the peer's
     * certificate.
     *
     * @remark The default value is `true`. More about this option:
     * https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
     *
     */
    bool SSLVerifyPeer = true;

    /**
     * @brief Define the SSL options for the libcurl handle.
     *
     * @remark See more info here: https://curl.haxx.se/libcurl/c/CURLOPT_SSL_OPTIONS.html.
     * The default option is all options `false`.
     *
     */
    CurlTransportSSLOptions SSLOptions;
  };

  /**
   * @brief Concrete implementation of an HTTP Transport that uses libcurl.
   *
   */
  class CurlTransport : public HttpTransport {
  private:
    CurlTransportOptions m_options;

  public:
    /**
     * @brief Construct a new Curl Transport object.
     *
     * @param options Optional parameter to override the default options.
     */
    CurlTransport(CurlTransportOptions const& options = CurlTransportOptions()) : m_options(options)
    {
    }

    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse
     *
     * @param context #Azure::Core::Context so that operation can be cancelled.
     * @param request an HTTP Request to be send.
     * @return unique ptr to an HTTP RawResponse.
     */
    std::unique_ptr<RawResponse> Send(Context const& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http

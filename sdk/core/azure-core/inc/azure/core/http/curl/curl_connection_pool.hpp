// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The curl connection pool provides the utilities for creating a new curl connection and to
 * keep a pool of connections to be re-used.
 */

#pragma once

#include "azure/core/http/curl/curl_connection.hpp"
#include "azure/core/http/http.hpp"

#include <curl/curl.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>

#ifdef TESTING_BUILD
// Define the class name that reads from ConnectionPool private members
namespace Azure { namespace Core { namespace Test {
  class TransportAdapter_connectionPoolTest_Test;
}}} // namespace Azure::Core::Test
#endif

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
    bool AllowBeast = false;
    bool NoRevoke = false;
    /*
    // Requires libcurl version >= 7.68
    bool NoPartialchain = false;
    // Requires libcurl version >= 7.70
    bool RevokeBestEffort = false;
    // Requires libcurl version >= 7.71
    bool NativeCa = false;
    */
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
   * @brief CURL HTTP connection pool makes it possible to re-use one curl connection to perform
   * more than one request. Use this component when connections are not re-used by default.
   *
   * This pool offers static methods and it is allocated statically. There can be only one
   * connection pool per application.
   */
  class CurlConnectionPool {
#ifdef TESTING_BUILD
    // Give access to private to this tests class
    friend class Azure::Core::Test::TransportAdapter_connectionPoolTest_Test;
#endif
  public:
    /**
     * @brief Mutex for accessing connection pool for thread-safe reading and writing.
     */
    static std::mutex ConnectionPoolMutex;

    /**
     * @brief Keeps an unique key for each host and creates a connection pool for each key.
     *
     * @detail This way getting a connection for a specific host can be done in O(1) instead of
     * looping a single connection list to find the first connection for the required host.
     *
     * @remark There might be multiple connections for each host.
     */
    static std::map<std::string, std::list<std::unique_ptr<CurlNetworkConnection>>>
        ConnectionPoolIndex;

    /**
     * @brief Finds a connection to be re-used from the connection pool.
     * @remark If there is not any available connection, a new connection is created.
     *
     * @param request HTTP request to get #CurlNetworkConnection for.
     *
     * @return #CurlNetworkConnection to use.
     */
    static std::unique_ptr<CurlNetworkConnection> GetCurlConnection(
        Request& request,
        CurlTransportOptions const& options);

    /**
     * @brief Moves a connection back to the pool to be re-used.
     *
     * @param connection CURL HTTP connection to add to the pool.
     * @param lastStatusCode The most recent HTTP status code received from the \p connection.
     */
    static void MoveConnectionBackToPool(
        std::unique_ptr<CurlNetworkConnection> connection,
        HttpStatusCode lastStatusCode);

    // Class can't have instances.
    CurlConnectionPool() = delete;

  private:
    /**
     * Review all connections in the pool and removes old connections that might be already
     * expired and closed its connection on server side.
     */
    static void CleanUp();

    static int32_t s_connectionCounter;
    static bool s_isCleanConnectionsRunning;
    // Removes all connections and indexes
    static void ClearIndex() { CurlConnectionPool::ConnectionPoolIndex.clear(); }

    // Makes possible to know the number of current connections in the connection pool for an
    // index
    static int64_t ConnectionsOnPool(std::string const& host)
    {
      auto& pool = CurlConnectionPool::ConnectionPoolIndex[host];
      return pool.size();
    };

    // Makes possible to know the number indexes in the pool
    static int64_t ConnectionsIndexOnPool()
    {
      return CurlConnectionPool::ConnectionPoolIndex.size();
    };
  };
}}} // namespace Azure::Core::Http

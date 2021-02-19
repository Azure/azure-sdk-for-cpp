// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The curl connection pool provides the utilities for creating a new curl connection and to
 * keep a pool of connections to be re-used.
 */

#pragma once

#include "azure/core/dll_import_export.hpp"
#include "azure/core/http/http.hpp"

#include "curl_connection_private.hpp"

#include <curl/curl.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>

#if defined(TESTING_BUILD)
// Define the class name that reads from ConnectionPool private members
namespace Azure { namespace Core { namespace Test {
  class CurlConnectionPool_connectionPoolTest_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http {

  /**
   * @brief CURL HTTP connection pool makes it possible to re-use one curl connection to perform
   * more than one request. Use this component when connections are not re-used by default.
   *
   * This pool offers static methods and it is allocated statically. There can be only one
   * connection pool per application.
   */
  class CurlConnectionPool {
#if defined(TESTING_BUILD)
    // Give access to private to this tests class
    friend class Azure::Core::Test::CurlConnectionPool_connectionPoolTest_Test;
#endif
  public:
    /**
     * @brief Mutex for accessing connection pool for thread-safe reading and writing.
     */
    AZ_CORE_DLLEXPORT static std::mutex ConnectionPoolMutex;

    /**
     * @brief Keeps an unique key for each host and creates a connection pool for each key.
     *
     * @details This way getting a connection for a specific host can be done in O(1) instead of
     * looping a single connection list to find the first connection for the required host.
     *
     * @remark There might be multiple connections for each host.
     */
    AZ_CORE_DLLEXPORT static std::
        map<std::string, std::list<std::unique_ptr<CurlNetworkConnection>>>
            ConnectionPoolIndex;

    /**
     * @brief Finds a connection to be re-used from the connection pool.
     * @remark If there is not any available connection, a new connection is created.
     *
     * @param request HTTP request to get #Azure::Core::Http::CurlNetworkConnection for.
     *
     * @return #Azure::Core::Http::CurlNetworkConnection to use.
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

    AZ_CORE_DLLEXPORT static int32_t s_connectionCounter;
    AZ_CORE_DLLEXPORT static bool s_isCleanConnectionsRunning;
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

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

#include <atomic>
#include <curl/curl.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>

#if defined(TESTING_BUILD)
// Define the class name that reads from ConnectionPool private members
namespace Azure { namespace Core { namespace Test {
  class CurlConnectionPool_connectionPoolTest_Test;
  class CurlConnectionPool_uniquePort_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http { namespace _detail {

  // In charge of calling the libcurl global functions for the Azure SDK
  struct CurlGlobalStateForAzureSdk;

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
    friend class Azure::Core::Test::CurlConnectionPool_uniquePort_Test;
#endif
  private:
    // The cttor and dttor of this member makes sure of calling the libcurl global init and cleanup
    AZ_CORE_DLLEXPORT static CurlGlobalStateForAzureSdk CurlGlobalState;

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
     * @param options The connection settings which includes host name and libcurl handle specific
     * configuration.
     * @param resetPool Request the pool to remove all current connections for the provided
     * options to force the creation of a new connection.
     *
     * @return #Azure::Core::Http::CurlNetworkConnection to use.
     */
    static std::unique_ptr<CurlNetworkConnection> ExtractOrCreateCurlConnection(
        Request& request,
        CurlTransportOptions const& options,
        bool resetPool = false);

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

    static void StopCleaner() { g_isCleanConnectionsRunning = false; }

  private:
    /**
     * Review all connections in the pool and removes old connections that might be already
     * expired and closed its connection on server side.
     */
    static void CleanUp();

    AZ_CORE_DLLEXPORT static uint64_t g_connectionCounter;
    AZ_CORE_DLLEXPORT static std::atomic<bool> g_isCleanConnectionsRunning;
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

  struct CurlGlobalStateForAzureSdk
  {
    CurlGlobalStateForAzureSdk() { curl_global_init(CURL_GLOBAL_ALL); }

    ~CurlGlobalStateForAzureSdk()
    {
      CurlConnectionPool::StopCleaner();
      curl_global_cleanup();
    }
  };

}}}} // namespace Azure::Core::Http::_detail

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief The curl connection pool provides the utilities for creating a new curl connection and to
 * keep a pool of connections to be re-used.
 */

#pragma once

#include "azure/core/dll_import_export.hpp"
#include "azure/core/http/http.hpp"
#include "curl_connection_private.hpp"

#include <azure/core/http/curl_transport.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#if defined(_azure_TESTING_BUILD)
// Define the class name that reads from ConnectionPool private members
namespace Azure { namespace Core { namespace Test {
  class CurlConnectionPool_connectionPoolTest_Test;
  class CurlConnectionPool_DISABLED_connectionPoolTest_Test;
  class CurlConnectionPool_uniquePort_Test;
  class CurlConnectionPool_connectionClose_Test;
  class SdkWithLibcurl_globalCleanUp_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http { namespace _detail {

  /**
   * @brief Global CURLSH share object for DNS and SSL session cache sharing across threads.
   * 
   * @details Per libcurl documentation, CURL_LOCK_DATA_DNS and CURL_LOCK_DATA_SSL_SESSION
   * are thread-safe when proper locking callbacks are provided. CURL_LOCK_DATA_CONNECT is
   * NOT thread-safe for concurrent access, so we only share DNS and SSL sessions.
   * 
   * Benefits:
   * - DNS cache shared across all handles (eliminates redundant lookups)
   * - SSL session IDs shared for faster TLS resume
   * - Significantly reduces latency from repeated DNS/TLS overhead
   */
  class GlobalCurlShareObject final {
  private:
    CURLSH* m_shareHandle;
    std::mutex m_shareLocks[CURL_LOCK_DATA_LAST];

    static void LockCallback(CURL* /*handle*/, curl_lock_data data, curl_lock_access /*access*/, void* userptr) {
      auto* shareObj = static_cast<GlobalCurlShareObject*>(userptr);
      shareObj->m_shareLocks[data].lock();
    }

    static void UnlockCallback(CURL* /*handle*/, curl_lock_data data, void* userptr) {
      auto* shareObj = static_cast<GlobalCurlShareObject*>(userptr);
      shareObj->m_shareLocks[data].unlock();
    }

  public:
    GlobalCurlShareObject() {
      m_shareHandle = curl_share_init();
      if (m_shareHandle) {
        // Share DNS cache across all handles
        curl_share_setopt(m_shareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        // Share SSL session IDs for faster TLS resume
        curl_share_setopt(m_shareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        // Set locking callbacks for thread safety
        curl_share_setopt(m_shareHandle, CURLSHOPT_LOCKFUNC, LockCallback);
        curl_share_setopt(m_shareHandle, CURLSHOPT_UNLOCKFUNC, UnlockCallback);
        curl_share_setopt(m_shareHandle, CURLSHOPT_USERDATA, this);
      }
    }

    ~GlobalCurlShareObject() {
      if (m_shareHandle) {
        curl_share_cleanup(m_shareHandle);
      }
    }

    CURLSH* GetShareHandle() const { return m_shareHandle; }

    static GlobalCurlShareObject& GetInstance() {
      static GlobalCurlShareObject instance;
      return instance;
    }

    // Prevent copying
    GlobalCurlShareObject(const GlobalCurlShareObject&) = delete;
    GlobalCurlShareObject& operator=(const GlobalCurlShareObject&) = delete;
  };

  /**
   * @brief Global pool of reusable CURL* handles matching WinHTTP's session handle approach.
   * 
   * @details WinHTTP uses ONE session handle for all requests, allowing Windows to manage
   * a process-wide connection pool. We mimic this by maintaining a large pool of CURL* handles
   * where each can cache connections. Pool size scaled to expected concurrency.
   * 
   * Strategy: Many handles (100) × moderate connections per handle (100) = extensive reuse
   */
  class GlobalCurlHandlePool final {
  private:
    std::mutex m_mutex;
    std::vector<CURL*> m_availableHandles;
    size_t m_maxPoolSize;
    std::atomic<size_t> m_totalHandlesCreated{0};

  public:
    GlobalCurlHandlePool() : m_maxPoolSize(100) {} // Match typical concurrency levels

    static GlobalCurlHandlePool& GetInstance() {
      static GlobalCurlHandlePool instance;
      return instance;
    }

    ~GlobalCurlHandlePool() {
      std::lock_guard<std::mutex> lock(m_mutex);
      for (auto* handle : m_availableHandles) {
        curl_easy_cleanup(handle);
      }
      m_availableHandles.clear();
    }

    /**
     * @brief Acquire a CURL* handle from the pool or create a new one.
     * @return A CURL* handle ready for configuration with shared DNS/SSL caches.
     */
    CURL* AcquireHandle() {
      CURL* handle = nullptr;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_availableHandles.empty()) {
          handle = m_availableHandles.back();
          m_availableHandles.pop_back();
          curl_easy_reset(handle);
        }
      }
      
      if (!handle) {
        // Create new handle outside lock
        handle = curl_easy_init();
        m_totalHandlesCreated++;
      }

      // Apply shared DNS and SSL session cache to all handles
      if (handle) {
        auto* shareHandle = GlobalCurlShareObject::GetInstance().GetShareHandle();
        if (shareHandle) {
          curl_easy_setopt(handle, CURLOPT_SHARE, shareHandle);
        }
      }
      
      return handle;
    }

    /**
     * @brief Return a CURL* handle to the pool for reuse.
     * @param handle The CURL* handle to return.
     */
    void ReleaseHandle(CURL* handle) {
      if (!handle) return;
      
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_availableHandles.size() < m_maxPoolSize) {
        m_availableHandles.push_back(handle);
      } else {
        curl_easy_cleanup(handle);
      }
    }

    /**
     * @brief Get current pool statistics.
     */
    size_t GetPoolSize() const {
      std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
      return m_availableHandles.size();
    }
    
    size_t GetTotalHandlesCreated() const {
      return m_totalHandlesCreated.load();
    }
  };

  /**
   * @brief CURL HTTP connection pool makes it possible to re-use one curl connection to perform
   * more than one request. Use this component when connections are not re-used by default.
   *
   * This pool offers static methods and it is allocated statically. There can be only one
   * connection pool per application.
   */
  class CurlConnectionPool final {
#if defined(_azure_TESTING_BUILD)
    // Give access to private to this tests class
    friend class Azure::Core::Test::CurlConnectionPool_connectionPoolTest_Test;
    friend class Azure::Core::Test::CurlConnectionPool_DISABLED_connectionPoolTest_Test;
    friend class Azure::Core::Test::CurlConnectionPool_uniquePort_Test;
    friend class Azure::Core::Test::CurlConnectionPool_connectionClose_Test;
    friend class Azure::Core::Test::SdkWithLibcurl_globalCleanUp_Test;
#endif

  public:
    ~CurlConnectionPool()
    {
      using namespace Azure::Core::Http::_detail;
      if (m_cleanThread.joinable())
      {
        {
          std::unique_lock<std::mutex> lock(ConnectionPoolMutex);
          // Remove all connections
          g_curlConnectionPool.ConnectionPoolIndex.clear();
        }
        // Signal clean thread to wake up
        ConditionalVariableForCleanThread.notify_one();
        // join thread
        m_cleanThread.join();
      }
      curl_global_cleanup();
    }

    /**
     * @brief Finds a connection to be re-used from the connection pool.
     * @remark If there is not any available connection, a new connection is created.
     *
     * @param request HTTP request to get #Azure::Core::Http::CurlNetworkConnection for.
     * @param options The connection settings which includes host name and libcurl handle specific
     * configuration.
     * @param connectionTimeoutOverride If greater than 0, specifies the override value for the
     * ConnectionTimeout value, specified in options.
     * @param resetPool Request the pool to remove all current connections for the provided
     * options to force the creation of a new connection.
     *
     * @return #Azure::Core::Http::CurlNetworkConnection to use.
     */
    std::unique_ptr<CurlNetworkConnection> ExtractOrCreateCurlConnection(
        Request& request,
        CurlTransportOptions const& options,
        std::chrono::milliseconds connectionTimeoutOverride = std::chrono::milliseconds{0},
        bool resetPool = false);

    /**
     * @brief Moves a connection back to the pool to be re-used.
     *
     * @param connection CURL HTTP connection to add to the pool.
     * @param httpKeepAlive The status of keep-alive behavior, based on HTTP protocol version and
     * the most recent response header received through the \p connection.
     */
    void MoveConnectionBackToPool(
        std::unique_ptr<CurlNetworkConnection> connection,
        bool httpKeepAlive);

    /**
     * @brief Keeps a unique key for each host and creates a connection pool for each key.
     *
     * @details This way getting a connection for a specific host can be done in O(1) instead of
     * looping a single connection list to find the first connection for the required host.
     *
     * @remark There might be multiple connections for each host.
     */
    std::unordered_map<std::string, std::list<std::unique_ptr<CurlNetworkConnection>>>
        ConnectionPoolIndex;

    std::mutex ConnectionPoolMutex;

    // This is used to put the cleaning pool thread to sleep and yet to be able to wake it if the
    // application finishes.
    std::condition_variable ConditionalVariableForCleanThread;

    AZ_CORE_DLLEXPORT static Azure::Core::Http::_detail::CurlConnectionPool g_curlConnectionPool;

    bool IsCleanThreadRunning = false;

  private:
    // private constructor to keep this as singleton.
    CurlConnectionPool() { curl_global_init(CURL_GLOBAL_ALL); }

    // Makes possible to know the number of current connections in the connection pool for an
    // index
    size_t ConnectionsOnPool(std::string const& host) { return ConnectionPoolIndex[host].size(); }

    std::thread m_cleanThread;
  };

}}}} // namespace Azure::Core::Http::_detail

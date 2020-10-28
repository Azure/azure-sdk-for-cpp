// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief The libcurl connection keeps the curl handle and performs the data transfer to the
 * network.
 */

#pragma once

#include <chrono>
#include <curl/curl.h>
#include <string>

namespace Azure { namespace Core { namespace Http {

  namespace Details {
    // 60 sec -> expired connection is when it waits for 60 sec or more and it's not re-used
    constexpr static int c_DefaultConnectionExpiredMilliseconds = 1000 * 60;
  } // namespace Details

  /**
   * @brief CURL HTTP connection.
   */
  class CurlConnection {
  private:
    CURL* m_handle;
    std::string m_host;
    std::chrono::steady_clock::time_point m_lastUseTime;

  public:
    /**
     * @Brief Construct CURL HTTP connection.
     *
     * @param host HTTP connection host name.
     */
    CurlConnection(std::string const& host) : m_handle(curl_easy_init()), m_host(host) {}

    /**
     * @brief Destructor.
     * @detail Cleans up CURL (invokes `curl_easy_cleanup()`).
     */
    ~CurlConnection() { curl_easy_cleanup(this->m_handle); }

    /**
     * @brief Get CURL handle.
     * @return CURL handle for the HTTP connection.
     */
    CURL* GetHandle() { return this->m_handle; }

    /**
     * @brief Get HTTP connection host.
     * @return HTTP connection host name.
     */
    std::string GetHost() const { return this->m_host; }

    /**
     * @brief Update last usage time for the connection.
     */
    void updateLastUsageTime() { this->m_lastUseTime = std::chrono::steady_clock::now(); }

    /**
     * @brief Checks whether this CURL connection is expired.
     * @return `true` if this connection is considered expired, `false` otherwise.
     */
    bool isExpired()
    {
      auto connectionOnWaitingTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - this->m_lastUseTime);
      return connectionOnWaitingTimeMs.count() >= Details::c_DefaultConnectionExpiredMilliseconds;
    }
  };
}}} // namespace Azure::Core::Http

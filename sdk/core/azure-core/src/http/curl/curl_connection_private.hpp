// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The libcurl connection keeps the curl handle and performs the data transfer to the
 * network.
 */

#pragma once

#include "azure/core/http/http.hpp"

#include <chrono>
#include <curl/curl.h>
#include <string>

namespace Azure { namespace Core { namespace Http {

  namespace Details {
    // libcurl CURL_MAX_WRITE_SIZE is 64k. Using same value for default uploading chunk size.
    // This can be customizable in the HttpRequest
    constexpr static int64_t DefaultUploadChunkSize = 1024 * 64;
    constexpr static auto DefaultLibcurlReaderSize = 1024;
    // Run time error template
    constexpr static const char* DefaultFailedToGetNewConnectionTemplate
        = "Fail to get a new connection for: ";
    constexpr static int DefaultMaxOpenNewConnectionIntentsAllowed = 10;
    // 90 sec -> cleaner wait time before next clean routine
    constexpr static int DefaultCleanerIntervalMilliseconds = 1000 * 90;
    // 60 sec -> expired connection is when it waits for 60 sec or more and it's not re-used
    constexpr static int DefaultConnectionExpiredMilliseconds = 1000 * 60;
  } // namespace Details

  /**
   * @brief Interface for the connection to the network with Curl.
   *
   * @remark This interface enables to mock the communication to the network with any behavior for
   * testing.
   *
   */
  class CurlNetworkConnection {
  public:
    /**
     * @brief Allow derived classes calling a destructor.
     *
     */
    virtual ~CurlNetworkConnection() = default;

    /**
     * @brief Get the Connection Properties Key object
     *
     */
    virtual std::string const& GetConnectionKey() const = 0;

    /**
     * @brief Update last usage time for the connection.
     */
    virtual void updateLastUsageTime() = 0;

    /**
     * @brief Checks whether this CURL connection is expired.
     */
    virtual bool isExpired() = 0;

    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or until
     * there is no more data to get from the socket.
     *
     */
    virtual int64_t ReadFromSocket(Context const& context, uint8_t* buffer, int64_t bufferSize) = 0;

    /**
     * @brief This method will use libcurl socket to write all the bytes from buffer.
     *
     */
    virtual CURLcode SendBuffer(Context const& context, uint8_t const* buffer, size_t bufferSize)
        = 0;
  };

  /**
   * @brief CURL HTTP connection.
   */
  class CurlConnection : public CurlNetworkConnection {
  private:
    CURL* m_handle;
    curl_socket_t m_curlSocket;
    std::chrono::steady_clock::time_point m_lastUseTime;
    std::string m_connectionKey;

  public:
    /**
     * @Brief Construct CURL HTTP connection.
     *
     * @param host HTTP connection host name.
     */
    CurlConnection(CURL* handle, std::string connectionPropertiesKey)
        : m_handle(handle), m_connectionKey(std::move(connectionPropertiesKey))
    {
      // Get the socket that libcurl is using from handle. Will use this to wait while
      // reading/writing
      // into wire
#if defined(_MSC_VER)
#pragma warning(push)
// C26812: The enum type 'CURLcode' is unscoped. Prefer 'enum class' over 'enum' (Enum.3)
#pragma warning(disable : 26812)
#endif
        auto result = curl_easy_getinfo(m_handle, CURLINFO_ACTIVESOCKET, &m_curlSocket);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        if (result != CURLE_OK)
        {
          throw Http::TransportException(
              "Broken connection. Couldn't get the active sockect for it."
              + std::string(curl_easy_strerror(result)));
        }
      }

      /**
       * @brief Destructor.
       * @details Cleans up CURL (invokes `curl_easy_cleanup()`).
       */
      ~CurlConnection() override { curl_easy_cleanup(this->m_handle); }

      std::string const& GetConnectionKey() const override { return this->m_connectionKey; }

      /**
       * @brief Update last usage time for the connection.
       */
      void updateLastUsageTime() override
      {
        this->m_lastUseTime = std::chrono::steady_clock::now();
      }

      /**
       * @brief Checks whether this CURL connection is expired.
       * @return `true` if this connection is considered expired, `false` otherwise.
       */
      bool isExpired() override
      {
        auto connectionOnWaitingTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - this->m_lastUseTime);
        return connectionOnWaitingTimeMs.count() >= Details::DefaultConnectionExpiredMilliseconds;
      }

      /**
       * @brief This function is used when working with streams to pull more data from the wire.
       * Function will try to keep pulling data from socket until the buffer is all written or until
       * there is no more data to get from the socket.
       *
       * @param context #Azure::Core::Context so that operation can be cancelled.
       * @param buffer ptr to buffer where to copy bytes from socket.
       * @param bufferSize size of the buffer and the requested bytes to be pulled from wire.
       * @return return the numbers of bytes pulled from socket. It can be less than what it was
       * requested.
       */
      int64_t ReadFromSocket(Context const& context, uint8_t* buffer, int64_t bufferSize) override;

      /**
       * @brief This method will use libcurl socket to write all the bytes from buffer.
       *
       * @remarks Hardcoded timeout is used in case a socket stop responding.
       *
       * @param context #Azure::Core::Context so that operation can be cancelled.
       * @param buffer ptr to the data to be sent to wire.
       * @param bufferSize size of the buffer to send.
       * @return CURL_OK when response is sent successfully.
       */
      CURLcode SendBuffer(Context const& context, uint8_t const* buffer, size_t bufferSize)
          override;
    };
}}} // namespace Azure::Core::Http

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief #HttpTransport implementation via CURL.
 */

#pragma once

#ifdef BUILD_CURL_HTTP_TRANSPORT_ADAPTER

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"

#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <type_traits>
#include <vector>

#ifdef TESTING_BUILD
// Define the class name that reads from ConnectionPool private members
namespace Azure { namespace Core { namespace Test {
  class TransportAdapter_connectionPoolTest_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http {

  namespace Details {
    // libcurl CURL_MAX_WRITE_SIZE is 64k. Using same value for default uploading chunk size.
    // This can be customizable in the HttpRequest
    constexpr static int64_t c_DefaultUploadChunkSize = 1024 * 64;
    constexpr static auto c_DefaultLibcurlReaderSize = 1024;
    // Run time error template
    constexpr static const char* c_DefaultFailedToGetNewConnectionTemplate
        = "Fail to get a new connection for: ";
    constexpr static int c_DefaultMaxOpenNewConnectionIntentsAllowed = 10;
    // 90 sec -> cleaner wait time before next clean routine
    constexpr static int c_DefaultCleanerIntervalMilliseconds = 1000 * 90;
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

  /**
   * @brief CURL HTTP connection pool makes it possible to re-use one curl connection to perform
   * more than one request. Use this component when connections are not re-used by default.
   *
   * This pool offers static methods and it is allocated statically. There can be only one
   * connection pool per application.
   */
  struct CurlConnectionPool
  {
#ifdef TESTING_BUILD
    // Give access to private to this tests class
    friend class Azure::Core::Test::TransportAdapter_connectionPoolTest_Test;
#endif

    /**
     * @brief Mutex for accessing connection pool for thread-safe reading and writing.
     */
    static std::mutex s_connectionPoolMutex;

    /**
     * @brief Keeps an unique key for each host and creates a connection pool for each key.
     *
     * @detail This way getting a connection for a specific host can be done in O(1) instead of
     * looping a single connection list to find the first connection for the required host.
     *
     * @remark There might be multiple connections for each host.
     */
    static std::map<std::string, std::list<std::unique_ptr<CurlConnection>>> s_connectionPoolIndex;

    /**
     * @brief Finds a connection to be re-used from the connection pool.
     * @remark If there is not any available connection, a new connection is created.
     *
     * @param request HTTP request to get #CurlConnection for.
     *
     * @return #CurlConnection to use.
     */
    static std::unique_ptr<CurlConnection> GetCurlConnection(Request& request);

    /**
     * @brief Moves a connection back to the pool to be re-used.
     *
     * @param connection CURL HTTP connection to add to the pool.
     * @param lastStatusCode The most recent HTTP status code received from the \p connection.
     */
    static void MoveConnectionBackToPool(
        std::unique_ptr<CurlConnection> connection,
        Http::HttpStatusCode lastStatusCode);

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
    static void ClearIndex() { CurlConnectionPool::s_connectionPoolIndex.clear(); }

    // Makes possible to know the number of current connections in the connection pool for an
    // index
    static int64_t ConnectionsOnPool(std::string const& host)
    {
      auto& pool = CurlConnectionPool::s_connectionPoolIndex[host];
      return pool.size();
    };

    // Makes possible to know the number indexes in the pool
    static int64_t ConnectionsIndexOnPool()
    {
      return CurlConnectionPool::s_connectionPoolIndex.size();
    };
  };

  /**
   * @brief Stateful component that controls sending an HTTP Request with libcurl over the wire.
   *
   * @remark This component does not use the classic libcurl easy interface to send and receive
   * bytes from the network using callbacks. Instead, `CurlSession` supports working with the custom
   * HTTP protocol option from libcurl to manually upload and download bytes from the network socket
   * using curl_easy_send() and curl_easy_recv().
   *
   * @remarks This component is expected to be used by an HTTP Transporter to ensure that
   * transporter to be reusable in multiple pipelines while every call to network is unique.
   */
  class CurlSession : public BodyStream {
  private:
    /*
     * Enum used by ResponseBufferParser to control the parsing internal state while building
     * the HTTP RawResponse
     *
     */
    enum class ResponseParserState
    {
      StatusLine,
      Headers,
      EndOfHeaders,
    };

    /**
     * @brief This is used to set the current state of a session.
     *
     * @remark The session needs to know what's the state on it when an exception occurs so the
     * connection is not moved back to the connection pool. When a new request is going to be sent,
     * the session will be in `PERFORM` until the request has been uploaded and a response code is
     * received from the server. At that point the state will change to `STREAMING`.
     * If there is any error before changing the state, the connection need to be cleaned up.
     *
     */
    enum class SessionState
    {
      PERFORM,
      STREAMING
    };

    /**
     * @brief stateful component used to read and parse a buffer to construct a valid HTTP
     * RawResponse.
     *
     * @remark It uses an internal string as a buffer to accumulate a response token (version, code,
     * header, etc.) until the next delimiter is found. Then it uses this string to keep building
     * the HTTP RawResponse.
     *
     * @remark Only status line and headers are parsed and built. Body is ignored by this
     * component. A libcurl session will use this component to build and return the HTTP
     * RawResponse with a body stream to the pipeline.
     */
    class ResponseBufferParser {
    private:
      /**
       * @brief Controls what the parser is expecting during the reading process
       *
       */
      ResponseParserState state;
      /**
       * @brief Unique ptr to a response. Parser will create an Initial-valid HTTP RawResponse and
       * then it will append headers to it. This response is moved to a different owner once
       * parsing is completed.
       *
       */
      std::unique_ptr<RawResponse> m_response;
      /**
       * @brief Indicates if parser has found the end of the headers and there is nothing left for
       * the HTTP RawResponse.
       *
       */
      bool m_parseCompleted;

      bool m_delimiterStartInPrevPosition;

      /**
       * @brief This buffer is used when the parsed buffer doesn't contain a completed token. The
       * content from the buffer will be appended to this buffer. Once that a delimiter is found,
       * the token for the HTTP RawResponse is taken from this internal sting if it contains data.
       *
       * @remark This buffer allows a libcurl session to use any size of buffer to read from a
       * socket while constructing an initial valid HTTP RawResponse. No matter if the response
       * from wire contains hundreds of headers, we can use only one fixed size buffer to parse it
       * all.
       *
       */
      std::string m_internalBuffer;

      /**
       * @brief This method is invoked by the Parsing process if the internal state is set to
       * status code. Function will get the status-line expected tokens until finding the end of
       * status line delimiter.
       *
       * @remark When the end of status line delimiter is found, this method will create the HTTP
       * RawResponse. The HTTP RawResponse is constructed by default with body type as Stream.
       *
       * @param buffer Points to a memory address with all or some part of a HTTP status line.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position from buffer.
       */
      int64_t BuildStatusCode(uint8_t const* const buffer, int64_t const bufferSize);

      /**
       * @brief This method is invoked by the Parsing process if the internal state is set to
       * headers. Function will keep adding headers to the HTTP RawResponse created before while
       * parsing an status line.
       *
       * @param buffer Points to a memory address with all or some part of a HTTP header.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position from buffer. When the returned
       * value is smaller than the body size, means there is part of the body response in the
       * buffer.
       */
      int64_t BuildHeader(uint8_t const* const buffer, int64_t const bufferSize);

    public:
      /**
       * @brief Construct a new RawResponse Buffer Parser object.
       *
       */
      ResponseBufferParser()
      {
        state = ResponseParserState::StatusLine;
        this->m_parseCompleted = false;
        this->m_delimiterStartInPrevPosition = false;
      }

      /**
       * @brief Parses the content of a buffer to construct a valid HTTP RawResponse. This method
       * is expected to be called over and over until it returns 0, indicating there is nothing
       * more to parse to build the HTTP RawResponse.
       *
       * @param buffer points to a memory area that contains, all or some part of an HTTP
       * response.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position. Returning a 0 means nothing was
       * parsed and it is likely that the HTTP RawResponse is completed. Returning the same value
       * as the buffer size means all buffer was parsed and the HTTP might be completed or not.
       * Returning a value smaller than the buffer size will likely indicate that the HTTP
       * RawResponse is completed and that the rest of the buffer contains part of the response
       * body.
       */
      int64_t Parse(uint8_t const* const buffer, int64_t const bufferSize);

      /**
       * @brief Indicates when the parser has completed parsing and building the HTTP RawResponse.
       *
       * @return `true` if parsing is completed. Otherwise `false`.
       */
      bool IsParseCompleted() const { return this->m_parseCompleted; }

      /**
       * @brief Moves the internal response to a different owner.
       *
       * @return Will move the response only if parsing is completed and if the HTTP RawResponse
       * was not moved before.
       */
      std::unique_ptr<RawResponse> GetResponse()
      {
        if (this->m_parseCompleted && this->m_response != nullptr)
        {
          return std::move(this->m_response);
        }
        return nullptr; // parse is not completed or response has been moved already.
      }
    };

    std::unique_ptr<CurlConnection> m_connection;

    /**
     * @brief libcurl socket abstraction used when working with streams.
     *
     */
    curl_socket_t m_curlSocket;

    /**
     * @brief The current state of the session.
     *
     * @remark The state of the session is used to determine if a connection can be moved back to
     * the connection pool or not. A connection can be re-used only when the session state is
     * `STREAMING` and the response has been read completely.
     *
     */
    SessionState m_sessionState;

    /**
     * @brief unique ptr for the HTTP RawResponse. The session is responsable for creating the
     * response once that an HTTP status line is received.
     *
     */
    std::unique_ptr<RawResponse> m_response;

    /**
     * @brief The HTTP Request for to be used by the session.
     *
     */
    Request& m_request;

    /**
     * @brief Control field to handle the case when part of HTTP response body was copied to the
     * inner buffer. When a libcurl stream tries to read part of the body, this field will help to
     * decide how much data to take from the inner buffer before pulling more data from network.
     *
     */
    int64_t m_bodyStartInBuffer;

    /**
     * @brief Control field to handle the number of bytes containing relevant data within the
     * internal buffer. This is because internal buffer can be set to be size N but after writing
     * from wire into it, it can be holding less then N bytes.
     *
     */
    int64_t m_innerBufferSize;

    bool m_isChunkedResponseType;

    /**
     * @brief This is a copy of the value of an HTTP response header `content-length`. The value
     * is received as string and parsed to size_t. This field avoid parsing the string header
     * every time from HTTP RawResponse.
     *
     * @remark This value is also used to avoid trying to read more data from network than what we
     * are expecting to.
     *
     */
    int64_t m_contentLength;

    /**
     * @brief For chunked responses, this field knows the size of the current chuck size server
     * will de sending
     *
     */
    int64_t m_chunkSize;

    int64_t m_sessionTotalRead = 0;

    /**
     * @brief Internal buffer from a session used to read bytes from a socket. This buffer is only
     * used while constructing an HTTP RawResponse without adding a body to it. Customers would
     * provide their own buffer to copy from socket when reading the HTTP body using streams.
     *
     */
    uint8_t m_readBuffer[Details::c_DefaultLibcurlReaderSize]; // to work with libcurl custom read.

    /**
     * @brief convenient function that indicates when the HTTP Request will need to upload a
     * payload or not.
     *
     * @return true if the HTTP Request will need to upload bytes to wire.
     *
     */
    bool isUploadRequest();

    /**
     * @brief Set up libcurl handle to behave as a specific HTTP Method.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetMethod();

    /**
     * @brief Creates a list of libcurl headers and set it up to CURLOPT_HTTPHEADER.
     *
     * @remark For an HTTP Request that requires uploading bytes to network, this method will set
     * the content-length header and will also set libcurl to avoid sending an expect; header to
     * only ask server if it is OK to upload the body.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetHeaders();

    /**
     * @brief Function used when working with Streams to manually write from the HTTP Request to
     * the wire.
     *
     * @param context #Context so that operation can be canceled.
     *
     * @return CURL_OK when response is sent successfully.
     */
    CURLcode SendRawHttp(Context const& context);

    /**
     * @brief Upload body.
     *
     * @param context #Context so that operation can be canceled.
     *
     * @return Curl code.
     */
    CURLcode UploadBody(Context const& context);

    /**
     * @brief This method will use libcurl socket to write all the bytes from buffer.
     *
     * @remarks Hardcoded timeout is used in case a socket stop responding.
     *
     * @param context #Context so that operation can be canceled.
     * @param buffer ptr to the data to be sent to wire.
     * @param bufferSize size of the buffer to send.
     * @return CURL_OK when response is sent successfully.
     */
    CURLcode SendBuffer(Context const& context, uint8_t const* buffer, size_t bufferSize);

    /**
     * @brief This function is used after sending an HTTP request to the server to read the HTTP
     * RawResponse from wire until the end of headers only.
     *
     * @param context #Context so that operation can be canceled.
     * @param reuseInternalBuffer Indicates whether the internal buffer should be reused.
     *
     * @return CURL_OK when an HTTP response is created.
     */
    void ReadStatusLineAndHeadersFromRawResponse(
        Context const& context,
        bool reuseInternalBuffer = false);

    /**
     * @brief Reads from inner buffer or from Wire until chunkSize is parsed and converted to
     * unsigned long long
     *
     * @param context #Context so that operation can be canceled.
     */
    void ParseChunkSize(Context const& context);

    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or until
     * there is no more data to get from the socket.
     *
     * @param context #Context so that operation can be canceled.
     * @param buffer ptr to buffer where to copy bytes from socket.
     * @param bufferSize size of the buffer and the requested bytes to be pulled from wire.
     * @return return the numbers of bytes pulled from socket. It can be less than what it was
     * requested.
     */
    int64_t ReadFromSocket(Context const& context, uint8_t* buffer, int64_t bufferSize);

    /**
     * @brief Last HTTP status code read.
     */
    Http::HttpStatusCode m_lastStatusCode;

    /**
     * @brief check whether an end of file has been reached.
     * @return `true` if end of file has been reached, `false` otherwise.
     */
    bool IsEOF()
    {
      auto eof = this->m_isChunkedResponseType ? this->m_chunkSize == 0
                                               : this->m_contentLength == this->m_sessionTotalRead;

      // `IsEOF` is called before trying to move a connection back to the connection pool.
      // If the session state is `PERFORM` it means the request could not complete an upload
      // operation (might have throw while uploading).
      // Connection should not be moved back to the connection pool on this scenario.
      return eof && m_sessionState != SessionState::PERFORM;
    }

  public:
    /**
     * @brief Construct a new Curl Session object. Init internal libcurl handler.
     *
     * @param request reference to an HTTP Request.
     */
    CurlSession(Request& request) : m_request(request)
    {
      this->m_connection = CurlConnectionPool::GetCurlConnection(this->m_request);
      this->m_bodyStartInBuffer = -1;
      this->m_innerBufferSize = Details::c_DefaultLibcurlReaderSize;
      this->m_isChunkedResponseType = false;
      this->m_sessionTotalRead = 0;
    }

    ~CurlSession() override
    {
      // mark connection as reusable only if entire response was read
      // If not, connection can't be reused because next Read will start from what it is currently
      // in the wire.
      // By not moving the connection back to the pool, it gets destroyed calling the connection
      // destructor to clean libcurl handle and close the connection.
      // IsEOF will also handle a connection that fail to complete an upload request.
      if (this->IsEOF())
      {
        CurlConnectionPool::MoveConnectionBackToPool(
            std::move(this->m_connection), this->m_lastStatusCode);
      }
    }

    /**
     * @brief Function will use the HTTP request received in constructor to perform a network call
     * based on the HTTP request configuration.
     *
     * @param context #Context so that operation can be canceled.
     * @return CURLE_OK when the network call is completed successfully.
     */
    CURLcode Perform(Context const& context);

    /**
     * @brief Moved the ownership of the HTTP RawResponse out of the session.
     *
     * @return the unique ptr to the HTTP RawResponse or null if the HTTP RawResponse is not yet
     * created or was moved before.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> GetResponse();

    /**
     * @brief Implement #BodyStream length.
     *
     * @return The size of the payload.
     */
    int64_t Length() const override { return this->m_contentLength; }

    /**
     * @brief Implement #BodyStream read. Calling this function pulls data from the wire.
     *
     * @param context #Context so that operation can be canceled.
     * @param buffer Buffer where data from wire is written to.
     * @param count The number of bytes to read from the network.
     * @return The actual number of bytes read from the network.
     */
    int64_t Read(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override;
  };

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

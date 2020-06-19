#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/policy.hpp"

#include <curl/curl.h>
#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  constexpr auto UploadSstreamPageSize = 1024 * 64;
  constexpr auto LibcurlReaderSize = 100;

  /**
   * @brief Statefull component that controls sending an HTTP Request with libcurl thru the wire and
   * parsing and building an HTTP Response.
   * This session supports the classic libcurl easy interface to send and receive bytes from network
   * using callbacks.
   * This session also supports working with the custom HTTP protocol option from libcurl to
   * manually upload and download bytes using a network socket. This implementation is used when
   * working with streams so customers can lazily pull data from netwok using an stream abstraction.
   *
   * @remarks This component is expected to be used by an HTTP Transporter to ensure that
   * transporter to be re usuable in multiple pipelines while every call to network is unique.
   */
  class CurlSession {
  private:
    /**
     * @brief Enum used by ResponseBufferParser to control the parsing internal state while building
     * the HTTP Response
     *
     */
    enum class ResponseParserState
    {
      StatusLine,
      Headers,
      EndOfHeaders,
    };

    /**
     * @brief stateful component used to read and parse a buffer to construct a valid HTTP Response.
     *
     * It uses an internal string as buffers to accumulate a response token (version, code, header,
     * etc) until the next delimiter is found. Then it uses this string to keep building the HTTP
     * Response.
     *
     * @remark Only status line and headers are parsed and built. Body is ignored by this component.
     * A libcurl session will use this component to build and return the HTTP Response with a body
     * stream to the pipeline.
     */
    class ResponseBufferParser {
    private:
      /**
       * @brief Controls what the parser is expecting during the reading process
       *
       */
      ResponseParserState state;
      /**
       * @brief Unique prt to a response. Parser will create an Initial-valid HTTP Response and then
       * it will append headers to it. This response is moved to a different owner once parsing is
       * completed.
       *
       */
      std::unique_ptr<Response> m_response;
      /**
       * @brief Indicates if parser has found the end of the headers and there is nothing left for
       * the HTTP Response.
       *
       */
      bool m_parseCompleted;

      /**
       * @brief This buffer is used when the parsed buffer doesn't contain a completed token. The
       * content from the buffer will be appended to this buffer. Once that a delimiter is found,
       * the token for the HTTP Response is taken from this internal sting if it contains data.
       *
       * @remark This buffer allows a libcurl session to use any size of buffer to read from a
       * socket while constructing an initial valid HTTP Response. No matter if the response from
       * wire contains hundreds of headers, we can use only one fixed size buffer to parse it all.
       *
       */
      std::string m_internalBuffer;

      /**
       * @brief This method is invoked by the Parsing process if the internal state is set to status
       * code. Function will get the status-line expected tokens until finding the end of status
       * line delimiter.
       *
       * @remark When the end of status line delimiter is found, this method will create the HTTP
       * Response. The HTTP Response is constructed by default with body type as Stream.
       *
       * @param buffer Points to a memory address with all or some part of a HTTP status line.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position from buffer.
       */
      size_t BuildStatusCode(uint8_t const* const buffer, size_t const bufferSize);

      /**
       * @brief This method is invoked by the Parsing process if the internal state is set to
       * headers. Function will keep adding headers to the HTTP Response created before while
       * parsing an status line.
       *
       * @param buffer Points to a memory address with all or some part of a HTTP header.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position from buffer. When the returned value
       * is smaller than the body size, means there is part of the body response in the buffer.
       */
      size_t BuildHeader(uint8_t const* const buffer, size_t const bufferSize);

    public:
      /**
       * @brief Construct a new Response Buffer Parser object.
       * Set the initial state and parsing completion.
       *
       */
      ResponseBufferParser()
      {
        state = ResponseParserState::StatusLine;
        this->m_parseCompleted = false;
      }

      // Parse contents of buffer to construct HttpResponse. Returns the index of the last parsed
      // possition. Return bufferSize when all buffer was used to parse
      /**
       * @brief Parses the content of a buffer to constuct a valid HTTP Response. This method is
       * expected to be called over and over until it returns 0, indicating there is nothing more to
       * parse to build the HTTP Response.
       *
       * @param buffer points to a memory area that contains, all or some part of an HTTP response.
       * @param bufferSize Indicates the size of the buffer.
       * @return Returns the index of the last parsed position. Returning a 0 means nothing was
       * parsed and it is likely that the HTTP Response is completed. Returning the same value as
       * the buffer size means all buffer was parsed and the HTTP might be completed or not.
       * Returning a value smaller than the buffer size will likely indicate that the HTTP Response
       * is completed and that the rest of the buffer contains part of the response body.
       */
      size_t Parse(uint8_t const* const buffer, size_t const bufferSize);

      /**
       * @brief Indicates when the parser has completed parsing and building the HTTP Response.
       *
       * @return true if parsing is completed. Otherwise false.
       */
      bool IsParseCompleted() const { return this->m_parseCompleted; }

      /**
       * @brief Moves the internal response to a different owner.
       *
       * @return Will move the response only if parsing is completed and if the HTTP Response was
       * not moved before.
       */
      std::unique_ptr<Response> GetResponse()
      {
        if (this->m_parseCompleted && this->m_response != nullptr)
        {
          return std::move(this->m_response);
        }
        return nullptr; // parse is not completed or response has been moved already.
      }
    };

    /**
     * @brief libcurl handle to be used in the session.
     *
     */
    CURL* m_pCurl;

    /**
     * @brief libcurl socket abstraction used when working with streams.
     *
     */
    curl_socket_t m_curlSocket;

    /**
     * @brief unique ptr for the HTTP Response. The session is responsable for creating the response
     * once that an HTTP status line is received.
     *
     */
    std::unique_ptr<Response> m_response;

    /**
     * @brief The HTTP Request for to be used by the session.
     *
     */
    Request& m_request;

    /**
     * @brief Controls the progress of a body buffer upload when using libcurl callbacks. Woks like
     * an offset to move the pointer to read the body from the HTTP Request on each callback.
     *
     */
    size_t uploadedBytes;

    /**
     * @brief Control field that gets true as soon as there is no more data to read from network. A
     * network socket will return 0 once we got the entire reponse.
     *
     */
    bool m_rawResponseEOF;

    /**
     * @brief Control field to handle the case when part of HTTP response body was copied to the
     * inner buffer. When a libcurl stream tries to read part of the body, this field will help to
     * decide how much data to take from the inner buffer before pulling more data from network.
     *
     */
    size_t m_bodyStartInBuffer;

    /**
     * @brief This is a copy of the value of an HTTP response header `content-length`. The value is
     * received as string and parsed to size_t. This field avoid parsing the string header everytime
     * from HTTP Response.
     *
     * @remark This value is also used to avoid trying to read more data from network than what we
     * are expecting to.
     *
     */
    uint64_t m_contentLength;

    /**
     * @brief Internal buffer from a session used to read bytes from a socket. This buffer is only
     * used while constructing an HTTP Response without adding a body to it. Customers would
     * provide their own buffer to copy from socket when reading the HTTP body using streams.
     *
     */
    uint8_t m_readBuffer[LibcurlReaderSize]; // to work with libcurl custom read.

    /**
     * @brief convenient function that indicates when the HTTP Request will need to upload a payload
     * or not.
     *
     * @return true if the HTTP Request will need to upload bytes to wire.
     *
     */
    bool isUploadRequest();

    /**
     * @brief Set up libcurl handle with a value for CURLOPT_URL.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetUrl();

    /**
     * @brief Set up libcurl handle with a value for CURLOPT_CONNECT_ONLY.
     *
     * @remark This configuration is required to enabled the custom upload/download from libcurl
     * easy interface.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetConnectOnly();

    /**
     * @brief Set up libcurl handle to behave as an specific HTTP Method.
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
     * @brief Set up libcurl callback functions for writing and user data. User data ptr for all
     * callbacks is set to reference the session object.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetWriteResponse();

    /**
     * @brief Set up libcurl callback functions for reading and user data. User data ptr for all
     * callbacks is set to reference the session object.
     *
     * @return returns the libcurl result after setting up.
     */
    CURLcode SetReadRequest();

    /**
     * @brief Function used when working with Streams to manually write from the HTTP Request to the
     * wire.
     *
     * @return CURL_OK when response is sent successfully.
     */
    CURLcode HttpRawSend();

    /**
     * @brief This method will use libcurl socket to write all the bytes from buffer.
     *
     * @remarks Hardcoded timeout is used in case a socket stop responding.
     *
     * @param buffer ptr to the data to be sent to wire.
     * @param bufferSize size of the buffer to send.
     * @return CURL_OK when response is sent successfully.
     */
    CURLcode SendBuffer(uint8_t* buffer, size_t bufferSize);

    /**
     * @brief This function is used after sending an HTTP request to the server to read the HTTP
     * Response from wire until the end of headers only.
     *
     * @return CURL_OK when an HTTP response is created.
     */
    CURLcode ReadStatusLineAndHeadersFromRawResponse();

    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or until
     * there is no more data to get from the socket.
     *
     * @param buffer prt to buffer where to copy bytes from socket.
     * @param bufferSize size of the buffer and the requested bytes to be pulled from wire.
     * @return return the numbers of bytes pulled from socket. It can be less than what it was
     * requested.
     */
    uint64_t ReadSocketToBuffer(uint8_t* buffer, size_t bufferSize);

  public:
    /**
     * @brief Construct a new Curl Session object. Init internal libcurl handler.
     *
     * @param request reference to an HTTP Request.
     */
    CurlSession(Request& request) : m_request(request)
    {
      this->m_pCurl = curl_easy_init();
      this->m_bodyStartInBuffer = 0;
    }

    /**
     * @brief Function will use the HTTP request received in constutor to perform a network call
     * based on the HTTP request configuration.
     *
     * @param context TBD
     * @return CURLE_OK when the network call is completed successfully.
     */
    CURLcode Perform(Context& context);

    /**
     * @brief Moved the ownership of the HTTP Response out of the session.
     *
     * @return the unique ptr to the HTTP Response or null if the HTTP Response is not yet created
     * or was moved before.
     */
    std::unique_ptr<Azure::Core::Http::Response> GetResponse();

    /**
     * @brief Helper method for reading with a Stream. Function will figure it out where to get
     * bytes from (either the libcurl socket of the internal buffer from session). The offset is
     * how stream controls how much it was already read.
     *
     * @param buffer ptr to a buffer where to write bytes from HTTP Response body.
     * @param bufferSize size of the buffer.
     * @param offset the number of bytes previously read.
     * @return the number of bytes read.
     */
    uint64_t ReadWithOffset(uint8_t* buffer, uint64_t bufferSize, uint64_t offset);
  };

  /**
   * @brief Concrete implementation of an HTTP Transport that uses libcurl.
   *
   */
  class CurlTransport : public HttpTransport {
  public:
    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP Response
     *
     * @param context TBD
     * @param request an HTTP Request to be send.
     * @return unique ptr to an HTTP Response.
     */
    std::unique_ptr<Response> Send(Context& context, Request& request) override;
  };

  /**
   * @brief concrete implementation of a body stream to read bytes for the HTTP body using libcurl
   * handler.
   */
  class CurlBodyStream : public Azure::Core::Http::BodyStream {
  private:
    /**
     * @brief length of the entire HTTP Response body.
     *
     */
    uint64_t m_length;

    /**
     * @brief reference to a Curl Session with all the configuration to be used to read from wire.
     *
     */
    CurlSession* m_curlSession;

    /**
     * @brief Numbers of bytes already read.
     *
     */
    uint64_t m_offset;

  public:
    /**
     * @brief Construct a new Curl Body Stream object.
     *
     * @param length size of the HTTP Response body.
     * @param curlSession reference to a libcurl session that contains the libcurl handler to be
     * used.
     */
    CurlBodyStream(uint64_t length, CurlSession* curlSession)
        : m_length(length), m_curlSession(curlSession), m_offset(0)
    {
    }

    ~CurlBodyStream()
    {
      if (this->m_curlSession != nullptr)
      {
        delete this->m_curlSession; // Session Destructor will cleanup libcurl handle
      }
    }

    /**
     * @brief Gets the length of the HTTP Response body.
     *
     * @return uint64_t
     */
    uint64_t Length() const override { return this->m_length; }

    /**
     * @brief Gets the number of bytes received on count from netwok. Copies the bytes to the
     * buffer.
     *
     * @param buffer ptr to a buffer where to copy bytes from network.
     * @param count number of bytes to copy from network into buffer.
     * @return the number of read and copied bytes from network to buffer.
     */
    uint64_t Read(uint8_t* buffer, uint64_t count) override
    {
      if (this->m_length == this->m_offset)
      {
        return 0;
      }
      // Read bytes from curl into buffer. As max as the length of Stream is allowed
      auto readCount = this->m_curlSession->ReadWithOffset(buffer, count, this->m_offset);
      this->m_offset += readCount;
      return readCount;
    }

    /**
     * @brief clean up heap. Removes the libcurl session and stream from the heap.
     *
     * @remark calling this method deletes the stream.
     *
     */
    void Close() override{};
  };

}}} // namespace Azure::Core::Http

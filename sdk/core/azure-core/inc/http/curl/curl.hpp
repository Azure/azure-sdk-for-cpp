#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/policy.hpp"

#include <curl/curl.h>
#include <type_traits>
#include <vector>

constexpr auto UPLOAD_STREAM_PAGE_SIZE = 1024 * 64;
constexpr auto LIBCURL_READER_SIZE = 100;

namespace Azure { namespace Core { namespace Http {

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
     * @brief Indicates if parser has found the end of the headers and there is nothing left for the
     * HTTP Response.
     *
     */
    bool parseCompleted;

    /**
     * @brief This buffer is used when the parsed buffer doesn't contain a completed token. The
     * content from the buffer will be appended to this buffer. Once that a delimiter is found, the
     * token for the HTTP Response is taken from this internal sting if it contains data.
     *
     * @remark This buffer allows a libcurl session to use any size of buffer to read from a socket
     * while constructing an initial valid HTTP Response. No matter if the response from wire
     * contains hundreds of headers, we can use only one fixed size buffer to parse it all.
     *
     */
    std::string internalBuffer;

    /**
     * @brief This method is invoked by the Parsing process if the internal state is set to status
     * code. Function will get the status-line expected tokens until finding the end of status line
     * delimiter.
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
     * @brief This method is invoked by the Parsing process if the internal state is set to headers.
     * Function will keep adding headers to the HTTP Response created before while parsing an status
     * line.
     *
     * @param buffer Points to a memory address with all or some part of a HTTP header.
     * @param bufferSize Indicates the size of the buffer.
     * @return Returns the index of the last parsed position from buffer. When the returned value is
     * smaller than the body size, means there is part of the body response in the buffer.
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
      parseCompleted = false;
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
     * @return Returns the index of the last parsed position. Returning a 0 means nothing was parsed
     * and it is likely that the HTTP Response is completed. Returning the same value as the buffer
     * size means all buffer was parsed and the HTTP might be completed or not. Returning a value
     * smaller than the buffer size will likely indicate that the HTTP Response is completed and
     * that the rest of the buffer contains part of the response body.
     */
    size_t Parse(uint8_t const* const buffer, size_t const bufferSize);

    /**
     * @brief Indicates when the parser has completed parsing and building the HTTP Response.
     *
     * @return true if parsing is completed. Otherwise false.
     */
    bool IsParseCompleted() const { return this->parseCompleted; }

    /**
     * @brief Moves the internal response to a different owner.
     *
     * @return Will move the response only if parsing is completed and if the HTTP Response was not
     * moved before.
     */
    std::unique_ptr<Response> GetResponse()
    {
      if (this->parseCompleted && this->m_response != nullptr)
      {
        return std::move(this->m_response);
      }
      return nullptr; // parse is not completed or response has been moved already.
    }
  };

  class CurlSession {
  private:
    CURL* m_pCurl;
    curl_socket_t m_curlSocket; // For Stream implementartion

    std::unique_ptr<Response> m_response;
    Request& m_request;
    size_t uploadedBytes; // controls a bodyBuffer upload

    // Reader control
    bool m_rawResponseEOF; // Set the end of a response to avoid keep reading a socket
    size_t m_bodyStartInBuffer; // Used for using innerBuffer as the start of body
    uint64_t m_contentLength;
    uint8_t m_readBuffer[LIBCURL_READER_SIZE]; // to work with libcurl custom read.

    // Headers
    static size_t WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    // Body from libcurl to httpResponse
    static size_t WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    // Body from httpRequest to libcurl
    static size_t ReadBodyCallBack(void* dst, size_t size, size_t nmemb, void* userdata);

    bool isUploadRequest()
    {
      return this->m_request.GetMethod() == HttpMethod::Put
          || this->m_request.GetMethod() == HttpMethod::Post;
    }

    // setHeaders()
    CURLcode SetUrl()
    {
      return curl_easy_setopt(m_pCurl, CURLOPT_URL, this->m_request.GetEncodedUrl().c_str());
    }
    CURLcode SetConnectOnly() { return curl_easy_setopt(m_pCurl, CURLOPT_CONNECT_ONLY, 1L); }
    CURLcode SetMethod()
    {
      HttpMethod method = this->m_request.GetMethod();
      if (method == HttpMethod::Get)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);
      }
      else if (method == HttpMethod::Put)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_PUT, 1L);
      }
      else if (method == HttpMethod::Head)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_NOBODY, 1L);
      }
      return CURLE_OK;
    }
    CURLcode SetHeaders()
    {
      auto headers = this->m_request.GetHeaders();
      if (headers.size() == 0)
      {
        return CURLE_OK;
      }

      // creates a slist for bulding curl headers
      struct curl_slist* headerList = NULL;

      // insert headers
      for (auto header : headers)
      {
        headerList = curl_slist_append(headerList, (header.first + ":" + header.second).c_str());
        if (headerList == NULL)
        {
          throw;
        }
      }

      if (isUploadRequest())
      {
        // set expect header for PUT and POST request. This disables libcurl to send only headers
        // and expect sever to return a Continue respond before posting body
        headerList = curl_slist_append(headerList, "Expect:");
        // inf header for payload size
        auto requestStream = this->m_request.GetBodyStream();
        auto size = requestStream != nullptr ? requestStream->Length()
                                             : this->m_request.GetBodyBuffer().size();
        auto result = curl_easy_setopt(m_pCurl, CURLOPT_INFILESIZE, (curl_off_t)size);
        if (result != CURLE_OK)
        {
          throw;
        }
      }

      // set all headers from slist
      return curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headerList);
    }
    CURLcode SetWriteResponse()
    {
      auto settingUp = curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, WriteHeadersCallBack);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_HEADERDATA, (void*)this);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      // TODO: set up cache size. user should be able to set it up
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteBodyCallBack);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }

      return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
    }
    // set callback for puting data into libcurl
    CURLcode SetReadRequest()
    {
      auto settingUp = curl_easy_setopt(m_pCurl, CURLOPT_UPLOAD, 1L);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, ReadBodyCallBack);
      this->uploadedBytes = 0; // restart control counter during setup
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_READDATA, (void*)this);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }

      return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
    }

    CURLcode HttpRawSend();
    CURLcode SendBuffer(uint8_t* buffer, size_t bufferSize);
    CURLcode ReadStatusLineAndHeadersFromRawResponse();
    uint64_t ReadSocketToBuffer(uint8_t* buffer, size_t bufferSize);

  public:
    CurlSession(Request& request) : m_request(request)
    {
      m_pCurl = curl_easy_init();
      m_bodyStartInBuffer = 0;
    }

    CURLcode Perform(Context& context);
    std::unique_ptr<Azure::Core::Http::Response> GetResponse() { return std::move(m_response); }
    // Api for CurlStream
    uint64_t ReadWithOffset(uint8_t* buffer, uint64_t bufferSize, uint64_t offset);
  };

  class CurlTransport : public HttpTransport {
  public:
    std::unique_ptr<Response> Send(Context& context, Request& request) override;

  }; // namespace Http

  // stream to be returned inside HTTP response when using curl
  // It keeps the ref to CurlTrasnport in order to close the handle once done
  class CurlBodyStream : public Azure::Core::Http::BodyStream {
  private:
    uint64_t m_length;
    CurlSession* m_curlSession;
    uint64_t m_offset;

  public:
    // length comes from http response header `content-length`
    CurlBodyStream(uint64_t length, CurlSession* curlSession)
        : m_length(length), m_curlSession(curlSession), m_offset(0)
    {
    }

    uint64_t Length() { return m_length; }

    uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t count)
    {
      if (m_length == m_offset)
      {
        return 0;
      }
      // Read bytes from curl into buffer. As max as the length of Stream is allowed
      auto readCount = this->m_curlSession->ReadWithOffset(buffer, count, m_offset);
      m_offset += readCount;
      return readCount;
    }

    void Close()
    {
      // call the cleanup from Session
      // Delete Session
      if (this->m_curlSession != nullptr)
      {
        delete this->m_curlSession;
      }
    };
  };
}}} // namespace Azure::Core::Http

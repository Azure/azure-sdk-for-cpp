// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP request and response functionality.
 */

#pragma once

#include "azure/core/case_insensitive_containers.hpp"
#include "azure/core/exception.hpp"
#include "azure/core/http/http_status_code.hpp"
#include "azure/core/http/raw_response.hpp"
#include "azure/core/internal/contract.hpp"
#include "azure/core/io/body_stream.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(TESTING_BUILD)
// Define the class used from tests to validate retry enabled
namespace Azure { namespace Core { namespace Test {
  class TestHttp_getters_Test;
  class TestHttp_query_parameter_Test;
  class TestHttp_RequestStartTry_Test;
  class TestURL_getters_Test;
  class TestURL_query_parameter_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http {

  /*********************  Exceptions  **********************/
  /**
   * @brief HTTP transport layer error.
   */
  class TransportException : public Azure::Core::RequestFailedException {
  public:
    /**
     * @brief An error while sending the HTTP request with the transport adapter.
     *
     * @remark The transport policy will throw this error whenever the transport adapter fail to
     * perform a request.
     *
     * @param message The error description.
     */
    explicit TransportException(std::string const& message)
        : Azure::Core::RequestFailedException(message)
    {
    }
  };

  /**
   * @brief Defines a range of bytes within an HTTP resource, starting at an `Offset` and ending at
   * `Offset + Length - 1` inclusively.
   *
   */
  struct HttpRange
  {
    /**
     * @brief The starting point of the HTTP Range.
     *
     */
    int64_t Offset = 0;

    /**
     * @brief The size of the HTTP Range.
     *
     */
    Azure::Nullable<int64_t> Length;
  };

  /**
   * HTTP request method.
   */
  enum class HttpMethod
  {
    Get, ///< GET
    Head, ///< HEAD
    Post, ///< POST
    Put, ///< PUT
    Delete, ///< DELETE
    Patch, ///< PATCH
  };

  /**
   * @brief Get a string representation for a value of #HttpMethod.
   *
   * @param method A value of #Azure::Core::Http::HttpMethod value.
   *
   * @return String name that corresponds to a value of #Azure::Core::Http::HttpMethod type.
   */
  inline std::string HttpMethodToString(const HttpMethod& method)
  {
    switch (method)
    {
      case HttpMethod::Get:
        return "GET";
      case HttpMethod::Head:
        return "HEAD";
      case HttpMethod::Post:
        return "POST";
      case HttpMethod::Put:
        return "PUT";
      case HttpMethod::Delete:
        return "DELETE";
      case HttpMethod::Patch:
        return "PATCH";
      default:
        return "";
    }
  }

  namespace Policies { namespace _internal {
    class RetryPolicy;
  }} // namespace Policies::_internal

  /**
   * @brief HTTP request.
   */
  class Request {
    friend class Azure::Core::Http::Policies::_internal::RetryPolicy;
#if defined(TESTING_BUILD)
    // make tests classes friends to validate set Retry
    friend class Azure::Core::Test::TestHttp_getters_Test;
    friend class Azure::Core::Test::TestHttp_query_parameter_Test;
    friend class Azure::Core::Test::TestHttp_RequestStartTry_Test;
    friend class Azure::Core::Test::TestURL_getters_Test;
    friend class Azure::Core::Test::TestURL_query_parameter_Test;
#endif

  private:
    HttpMethod m_method;
    Url m_url;
    CaseInsensitiveMap m_headers;
    CaseInsensitiveMap m_retryHeaders;

    Azure::Core::IO::BodyStream* m_bodyStream;

    // flag to know where to insert header
    bool m_retryModeEnabled{false};
    bool m_isDownloadViaStream;

    // Expected to be called by a Retry policy to reset all headers set after this function was
    // previously called
    void StartTry();

  public:
    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream #Azure::Core::IO::BodyStream.
     * @param downloadViaStream A boolean value indicating whether download should happen via
     * stream.
     */
    explicit Request(
        HttpMethod httpMethod,
        Url url,
        Azure::Core::IO::BodyStream* bodyStream,
        bool downloadViaStream)
        : m_method(std::move(httpMethod)), m_url(std::move(url)), m_bodyStream(bodyStream),
          m_retryModeEnabled(false), m_isDownloadViaStream(downloadViaStream)
    {
    }

    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream #Azure::Core::IO::BodyStream.
     */
    explicit Request(HttpMethod httpMethod, Url url, Azure::Core::IO::BodyStream* bodyStream)
        : Request(httpMethod, std::move(url), bodyStream, false)
    {
    }

    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param downloadViaStream A boolean value indicating whether download should happen via
     * stream.
     */
    explicit Request(HttpMethod httpMethod, Url url, bool downloadViaStream);

    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     */
    explicit Request(HttpMethod httpMethod, Url url);

    /**
     * @brief Set an HTTP header to the #Azure::Core::Http::Request.
     *
     * @remark If the header key does not exists, it is added.
     *
     *
     * @param name The name for the header to be set or added.
     * @param value The value for the header to be set or added.
     *
     * @throw if \p name is an invalid header key.
     */
    void SetHeader(std::string const& name, std::string const& value);

    /**
     * @brief Remove an HTTP header.
     *
     * @param name HTTP header name.
     */
    void RemoveHeader(std::string const& name);

    // Methods used by transport layer (and logger) to send request
    /**
     * @brief Get HTTP method.
     */
    HttpMethod GetMethod() const;

    /**
     * @brief Get HTTP headers.
     */
    CaseInsensitiveMap GetHeaders() const;

    /**
     * @brief Get HTTP body as #Azure::Core::IO::BodyStream.
     */
    Azure::Core::IO::BodyStream* GetBodyStream() { return this->m_bodyStream; }

    /**
     * @brief Get the list of headers prior to HTTP body.
     */
    std::string GetHeadersAsString() const;

    /**
     * @brief A value indicating whether download is happening via stream.
     */
    bool IsDownloadViaStream() { return this->m_isDownloadViaStream; }

    /**
     * @brief Get URL.
     */
    Url& GetUrl() { return this->m_url; }

    /**
     * @brief Get URL.
     */
    Url const& GetUrl() const { return this->m_url; }
  };

  namespace _detail {
    struct RawResponseHelpers
    {
      /**
       * @brief Insert a header into \p headers checking that \p headerName does not contain invalid
       * characters.
       *
       * @param headers The headers map where to insert header.
       * @param headerName The header name for the header to be inserted.
       * @param headerValue The header value for the header to be inserted.
       *
       * @throw if \p headerName is invalid.
       */
      static void InsertHeaderWithValidation(
          CaseInsensitiveMap& headers,
          std::string const& headerName,
          std::string const& headerValue);

      static void inline SetHeader(
          Azure::Core::Http::RawResponse& response,
          uint8_t const* const first,
          uint8_t const* const last)
      {
        // get name and value from header
        auto start = first;
        auto end = std::find(start, last, ':');

        if (end == last)
        {
          throw std::invalid_argument("Invalid header. No delimiter ':' found.");
        }

        // Always toLower() headers
        auto headerName
            = Azure::Core::_internal::StringExtensions::ToLower(std::string(start, end));
        start = end + 1; // start value
        while (start < last && (*start == ' ' || *start == '\t'))
        {
          ++start;
        }

        end = std::find(start, last, '\r');
        auto headerValue = std::string(start, end); // remove \r

        response.SetHeader(headerName, headerValue);
      }
    };
  } // namespace _detail

}}} // namespace Azure::Core::Http

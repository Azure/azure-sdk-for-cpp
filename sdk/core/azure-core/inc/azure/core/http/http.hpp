// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP request and response functionality.
 */

#pragma once

#include "azure/core/exception.hpp"
#include "azure/core/http/body_stream.hpp"
#include "azure/core/internal/contract.hpp"
#include "azure/core/nullable.hpp"

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
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http {

  namespace Details {
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
    void InsertHeaderWithValidation(
        std::map<std::string, std::string>& headers,
        std::string const& headerName,
        std::string const& headerValue);
  } // namespace Details

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
   * @brief An invalid header key name in #Azure::Core::Http::Request or
   * #Azure::Core::Http::RawResponse.
   *
   */
  class InvalidHeaderException : public Azure::Core::RequestFailedException {
  public:
    /**
     * @brief An invalid header key name detected in the HTTP request or response.
     *
     * @param message The error description.
     */
    explicit InvalidHeaderException(std::string const& message)
        : Azure::Core::RequestFailedException(message)
    {
    }
  };

  /**
   * @brief Defines the possible HTTP status codes.
   */
  enum class HttpStatusCode
  {
    /// No HTTP status code.
    None = 0,

    // === 1xx (information) Status Codes: ===
    Continue = 100, ///< HTTP 100 Continue.
    SwitchingProtocols = 101, ///< HTTP 101 Switching Protocols.
    Processing = 102, ///< HTTP 102 Processing.
    EarlyHints = 103, ///< HTTP 103 Early Hints.

    // === 2xx (successful) Status Codes: ===
    Ok = 200, ///< HTTP 200 OK.
    Created = 201, ///< HTTP 201 Created.
    Accepted = 202, ///< HTTP 202 Accepted.
    NonAuthoritativeInformation = 203, ///< HTTP 203 Non-Authoritative Information.
    NoContent = 204, ///< HTTP 204 No Content.
    ResetContent = 205, ///< HTTP 205 Rest Content.
    PartialContent = 206, ///< HTTP 206 Partial Content.
    MultiStatus = 207, ///< HTTP 207 Multi-Status.
    AlreadyReported = 208, ///< HTTP 208 Already Reported.
    IMUsed = 226, ///< HTTP 226 IM Used.

    // === 3xx (redirection) Status Codes: ===
    MultipleChoices = 300, ///< HTTP 300 Multiple Choices.
    MovedPermanently = 301, ///< HTTP 301 Moved Permanently.
    Found = 302, ///< HTTP 302 Found.
    SeeOther = 303, ///< HTTP 303 See Other.
    NotModified = 304, ///< HTTP 304 Not Modified.
    UseProxy = 305, ///< HTTP 305 Use Proxy.
    TemporaryRedirect = 307, ///< HTTP 307 Temporary Redirect.
    PermanentRedirect = 308, ///< HTTP 308 Permanent Redirect.

    // === 4xx (client error) Status Codes: ===
    BadRequest = 400, ///< HTTP 400 Bad Request.
    Unauthorized = 401, ///< HTTP 401 Unauthorized.
    PaymentRequired = 402, ///< HTTP 402 Payment Required.
    Forbidden = 403, ///< HTTP 403 Forbidden.
    NotFound = 404, ///< HTTP 404 Not Found.
    MethodNotAllowed = 405, ///< HTTP 405 Method Not Allowed.
    NotAcceptable = 406, ///< HTTP 406 Not Acceptable.
    ProxyAuthenticationRequired = 407, ///< HTTP 407 Proxy Authentication Required.
    RequestTimeout = 408, ///< HTTP 408 Request Timeout.
    Conflict = 409, ///< HTTP 409 Conflict.
    Gone = 410, ///< HTTP 410 Gone.
    LengthRequired = 411, ///< HTTP 411 Length Required.
    PreconditionFailed = 412, ///< HTTP 412 Precondition Failed.
    PayloadTooLarge = 413, ///< HTTP 413 Payload Too Large.
    UriTooLong = 414, ///< HTTP 414 URI Too Long.
    UnsupportedMediaType = 415, ///< HTTP 415 Unsupported Media Type.
    RangeNotSatisfiable = 416, ///< HTTP 416 Range Not Satisfiable.
    ExpectationFailed = 417, ///< HTTP 417 Expectation Failed.
    MisdirectedRequest = 421, ///< HTTP 421 Misdirected Request.
    UnprocessableEntity = 422, ///< HTTP 422 Unprocessable Entity.
    Locked = 423, ///< HTTP 423 Locked.
    FailedDependency = 424, ///< HTTP 424 Failed Dependency.
    TooEarly = 425, ///< HTTP 425 Too Early.
    UpgradeRequired = 426, ///< HTTP 426 Upgrade Required.
    PreconditionRequired = 428, ///< HTTP 428 Precondition Required.
    TooManyRequests = 429, ///< HTTP 429 Too Many Requests.
    RequestHeaderFieldsTooLarge = 431, ///< HTTP 431 Request Header Fields Too Large.
    UnavailableForLegalReasons = 451, ///< HTTP 451 Unavailable For Legal Reasons.

    // === 5xx (server error) Status Codes: ===
    InternalServerError = 500, ///< HTTP 500 Internal Server Error.
    NotImplemented = 501, ///< HTTP 501 Not Implemented.
    BadGateway = 502, ///< HTTP 502 Bad Gateway.
    ServiceUnavailable = 503, ///< HTTP 503 Unavailable.
    GatewayTimeout = 504, ///< HTTP 504 Gateway Timeout.
    HttpVersionNotSupported = 505, ///< HTTP 505 HTTP Version Not Supported.
    VariantAlsoNegotiates = 506, ///< HTTP 506 Variant Also Negotiates.
    InsufficientStorage = 507, ///< HTTP 507 Insufficient Storage.
    LoopDetected = 508, ///< HTTP 508 Loop Detected.
    NotExtended = 510, ///< HTTP 510 Not Extended.
    NetworkAuthenticationRequired = 511, ///< HTTP 511 Network Authentication Required.
  };

  /**
   * @brief Defines a range of bytes within an HTTP resource, starting at an `Offset` and ending at
   * `Offset + Length - 1` inclusively.
   *
   */
  struct Range
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
    Azure::Core::Nullable<int64_t> Length;
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

  /**
   * Type of HTTP response body.
   */
  enum class BodyType
  {
    Buffer, ///< Buffer.
    Stream, ///< Stream.
  };

  /**
   * @brief Url represents the location where a request will be performed.
   * It can be parsed and initialized from a string that contains all URL components (scheme, host,
   * path, etc.). Authority is not currently supported.
   */
  class Url {
  private:
    std::string m_scheme;
    std::string m_host;
    uint16_t m_port{0};
    std::string m_encodedPath;
    // query parameters are all encoded
    std::map<std::string, std::string> m_encodedQueryParameters;

    // List of default non-URL-encode chars. While URL encoding a string, do not escape any chars in
    // this set.
    const static std::unordered_set<unsigned char> defaultNonUrlEncodeChars;

  public:
    /**
     * @brief Decodes \p value by transforming all escaped characters to it's non-encoded value.
     *
     * @param value URL-encoded string.
     * @return std::string with non-URL encoded values.
     */
    static std::string Decode(const std::string& value);

    /**
     * @brief Encodes \p value by escaping characters to the form of %HH where HH are hex digits.
     *
     * @remark \p doNotEncodeSymbols arg can be used to explicitly ask this function to skip
     * characters from encoding. For instance, using this `= -` input would prevent encoding `=`, `
     * ` and `-`.
     *
     * @param value Non URL-encoded string.
     * @param doNotEncodeSymbols A string consisting of characters that do not need to be encoded.
     * @return std::string
     */
    static std::string Encode(const std::string& value, const std::string& doNotEncodeSymbols = "");

    /**
     * @brief Constructs a new, empty URL object.
     *
     */
    Url() {}

    /**
     * @brief Construct a URL from a URL-encoded string.
     *
     * @param encodedUrl URL string that has all its expected parts already URL-encoded.
     */
    explicit Url(const std::string& encodedUrl);

    /************* Builder Url functions ****************/
    /******** API for building Url from scratch. Override state ********/

    /**
     * @brief Set URL scheme.
     *
     * @param scheme URL scheme.
     */
    void SetScheme(const std::string& scheme) { m_scheme = scheme; }

    /**
     * @brief Set URL host.
     *
     * @param host URL host.
     */
    void SetHost(const std::string& encodedHost) { m_host = encodedHost; }

    /**
     * @brief Set URL port.
     *
     * @param port URL port.
     */
    void SetPort(uint16_t port) { m_port = port; }

    /**
     * @brief Set URL path.
     *
     * @param path URL path.
     */
    void SetPath(const std::string& encodedPath) { m_encodedPath = encodedPath; }

    /**
     * @brief Set the query parameters from an existing query parameter map.
     *
     * @remark Keys and values in \p queryParameters are expected to be URL-encoded.
     *
     * @param queryParameters
     */
    void SetQueryParameters(std::map<std::string, std::string> queryParameters)
    {
      // creates a copy and discard previous
      m_encodedQueryParameters = std::move(queryParameters);
    }

    // ===== APIs for mutating URL state: ======

    /**
     * @brief Append an element of URL path.
     *
     * @param path URL path element to append.
     */
    void AppendPath(const std::string& encodedPath)
    {
      if (!m_encodedPath.empty() && m_encodedPath.back() != '/')
      {
        m_encodedPath += '/';
      }
      m_encodedPath += encodedPath;
    }

    /**
     * @brief The value of a query parameter is expected to be non-URL-encoded and, by default, it
     * will be encoded before adding to the URL. Use \p isValueEncoded = true when the
     * value is already encoded.
     *
     * @remark This function overrides the value of existing query parameters.
     *
     * @param encodedKey Name of the query parameter, already encoded.
     * @param encodedValue Value of the query parameter, already encoded.
     */
    void AppendQueryParameter(const std::string& encodedKey, const std::string& encodedValue)
    {
      m_encodedQueryParameters[encodedKey] = encodedValue;
    }

    /**
     * @brief Finds the first '?' symbol and parses everything after it as query parameters.
     * separated by '&'.
     *
     * @param encodedQueryParameters String containing one or more query parameters.
     */
    void AppendQueryParameters(const std::string& encodedQueryParameters);

    /**
     * @brief Removes an existing query parameter.
     *
     * @param encodedKey The name of the query parameter to be removed.
     */
    void RemoveQueryParameter(const std::string& encodedKey)
    {
      m_encodedQueryParameters.erase(encodedKey);
    }

    /************** API to read values from Url ***************/
    /**
     * @brief Get URL host.
     */
    const std::string& GetHost() const { return m_host; }

    /**
     * @brief Gets the URL path.
     *
     * @return const std::string&
     */
    const std::string& GetPath() const { return m_encodedPath; }

    /**
     * @brief Get the port number set for the URL.
     *
     * @remark If the port was not set for the url, the returned port is 0. An HTTP request cannot
     * be performed to port zero, an HTTP client is expected to set the default port depending on
     * the request's schema when the port was not defined in the URL.
     *
     * @return The port number from the URL.
     */
    uint16_t GetPort() const { return m_port; }

    /**
     * @brief Provides a copy to the list of query parameters from the URL.
     *
     * @remark The query parameters are URL-encoded.
     *
     * @return const std::map<std::string, std::string>&
     */
    std::map<std::string, std::string> GetQueryParameters() const
    {
      return m_encodedQueryParameters;
    }

    /**
     * @brief Gets the path and query parameters.
     *
     * @return std::string The string is URL encoded.
     */
    std::string GetRelativeUrl() const;

    /**
     * @brief Gets Scheme, host, path and query parameters.
     *
     * @return std::string The string is URL encoded.
     */
    std::string GetAbsoluteUrl() const;
  };

  /**
   * @brief HTTP request.
   */
  class Request {
    friend class RetryPolicy;
#if defined(TESTING_BUILD)
    // make tests classes friends to validate set Retry
    friend class Azure::Core::Test::TestHttp_getters_Test;
    friend class Azure::Core::Test::TestHttp_query_parameter_Test;
#endif

  private:
    HttpMethod m_method;
    Url m_url;
    std::map<std::string, std::string> m_headers;
    std::map<std::string, std::string> m_retryHeaders;

    BodyStream* m_bodyStream;

    // flag to know where to insert header
    bool m_retryModeEnabled{false};
    bool m_isDownloadViaStream;

    // This value can be used to override the default value that an http transport adapter uses to
    // read and upload chunks of data from the payload body stream. If it is not set, the transport
    // adapter will decide chunk size.
    int64_t m_uploadChunkSize = 0;

  public:
    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream #Azure::Core::Http::BodyStream.
     * @param downloadViaStream A boolean value indicating whether download should happen via
     * stream.
     */
    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream, bool downloadViaStream)
        : m_method(std::move(httpMethod)), m_url(std::move(url)), m_bodyStream(bodyStream),
          m_retryModeEnabled(false), m_isDownloadViaStream(downloadViaStream)
    {
    }

    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream #Azure::Core::Http::BodyStream.
     */
    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream)
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
    explicit Request(HttpMethod httpMethod, Url url, bool downloadViaStream)
        : Request(
            httpMethod,
            std::move(url),
            NullBodyStream::GetNullBodyStream(),
            downloadViaStream)
    {
    }

    /**
     * @brief Construct an #Azure::Core::Http::Request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     */
    explicit Request(HttpMethod httpMethod, Url url)
        : Request(httpMethod, std::move(url), NullBodyStream::GetNullBodyStream(), false)
    {
    }

    /**
     * @brief Add HTTP header to the #Azure::Core::Http::Request.
     *
     * @param name The name for the header to be added.
     * @param value The value for the header to be added.
     *
     * @throw if \p name is an invalid header key.
     */
    void AddHeader(std::string const& name, std::string const& value);

    /**
     * @brief Remove an HTTP header.
     *
     * @param name HTTP header name.
     */
    void RemoveHeader(std::string const& name);

    /**
     * @brief Set upload chunk size.
     *
     * @param size Upload chunk size.
     */
    void SetUploadChunkSize(int64_t size) { this->m_uploadChunkSize = size; }

    // Methods used by transport layer (and logger) to send request
    /**
     * @brief Get HTTP method.
     */
    HttpMethod GetMethod() const;

    /**
     * @brief Get HTTP headers.
     */
    std::map<std::string, std::string> GetHeaders() const;

    /**
     * @brief Get HTTP body as #Azure::Core::Http::BodyStream.
     */
    BodyStream* GetBodyStream() { return this->m_bodyStream; }

    /**
     * @brief Get the list of headers prior to HTTP body.
     */
    std::string GetHeadersAsString() const;

    /**
     * @brief Get HTTP message prior to HTTP body.
     */
    std::string GetHTTPMessagePreBody() const;

    /**
     * @brief Get upload chunk size.
     */
    int64_t GetUploadChunkSize() { return this->m_uploadChunkSize; }

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
    // Expected to be called by a Retry policy to reset all headers set after this function was
    // previously called
    void StartTry();
  };

  /**
   * @brief Raw HTTP response.
   */
  class RawResponse {

  private:
    int32_t m_majorVersion;
    int32_t m_minorVersion;
    HttpStatusCode m_statusCode;
    std::string m_reasonPhrase;
    std::map<std::string, std::string> m_headers;

    std::unique_ptr<BodyStream> m_bodyStream;
    std::vector<uint8_t> m_body;

    explicit RawResponse(
        int32_t majorVersion,
        int32_t minorVersion,
        HttpStatusCode statusCode,
        std::string const& reasonPhrase,
        std::unique_ptr<BodyStream> BodyStream)
        : m_majorVersion(majorVersion), m_minorVersion(minorVersion), m_statusCode(statusCode),
          m_reasonPhrase(reasonPhrase), m_bodyStream(std::move(BodyStream))
    {
    }

  public:
    /**
     * @brief Construct raw HTTP response.
     *
     * @param majorVersion HTTP protocol version major number.
     * @param minorVersion HTTP protocol version minor number.
     * @param statusCode HTTP status code.
     * @param reasonPhrase HTP reason phrase.
     */
    explicit RawResponse(
        int32_t majorVersion,
        int32_t minorVersion,
        HttpStatusCode statusCode,
        std::string const& reasonPhrase)
        : RawResponse(majorVersion, minorVersion, statusCode, reasonPhrase, nullptr)
    {
    }

    /**
     * @brief Copy a raw response to construct a new one.
     *
     * @remark The body stream won't be copied.
     *
     * @param response A reference for copying the raw response.
     */
    RawResponse(RawResponse const& response)
        : RawResponse(
            response.m_majorVersion,
            response.m_minorVersion,
            response.m_statusCode,
            response.m_reasonPhrase)
    {
      // Copy body
      m_body = response.GetBody();
    }

    // ===== Methods used to build HTTP response =====

    /**
     * @brief Add HTTP header to the #Azure::Core::Http::RawResponse.
     *
     * @remark The \p name must contain valid header name characters (RFC 7230).
     *
     * @param name The name for the header to be added.
     * @param value The value for the header to be added.
     *
     * @throw if \p name contains invalid characters.
     */
    void AddHeader(std::string const& name, std::string const& value);

    /**
     * @brief Add HTTP header to the #Azure::Core::Http::RawResponse.
     *
     * @remark The \p header must contain valid header name characters (RFC 7230).
     * @remark Header name, value and delimiter are expected to be in \p header.
     *
     * @param header The complete header to be added, in the form "name:value".
     *
     * @throw if \p header has an invalid header name or if the delimiter is missing.
     */
    void AddHeader(std::string const& header);

    /**
     * @brief Add HTTP header to the #Azure::Core::Http::RawResponse.
     *
     * @remark The string referenced by \p first and \p last must contain valid header name
     * characters (RFC 7230).
     * @remark Header name, value and delimiter are expected to be in the string referenced by \p
     * first and \p last, in the form "name:value".
     *
     * @param first Reference to the start of an std::string.
     * @param last Reference to the end of an std::string.
     *
     * @throw if the string referenced by \p first and \p last contains an invalid header name or if
     * the delimiter is missing.
     */
    void AddHeader(uint8_t const* const first, uint8_t const* const last);

    /**
     * @brief Set #Azure::Core::Http::BodyStream for this HTTP response.
     *
     * @param stream #Azure::Core::Http::BodyStream.
     */
    void SetBodyStream(std::unique_ptr<BodyStream> stream);

    /**
     * @brief Set HTTP response body for this HTTP response.
     *
     * @param body HTTP response body bytes.
     */
    void SetBody(std::vector<uint8_t> body) { this->m_body = std::move(body); }

    // adding getters for version and stream body. Clang will complain on Mac if we have unused
    // fields in a class

    /**
     * @brief Get major number of the HTTP response protocol version.
     */
    int32_t GetMajorVersion() const { return this->m_majorVersion; }

    /**
     * @brief Get minor number of the HTTP response protocol version.
     */
    int32_t GetMinorVersion() const { return this->m_minorVersion; }

    /**
     * @brief Get HTTP status code of the HTTP response.
     */
    HttpStatusCode GetStatusCode() const;

    /**
     * @brief Get HTTP reason phrase code of the HTTP response.
     */
    std::string const& GetReasonPhrase() const;

    /**
     * @brief Get HTTP response headers.
     */
    std::map<std::string, std::string> const& GetHeaders() const;

    /**
     * @brief Get HTTP response body as #Azure::Core::Http::BodyStream.
     */
    std::unique_ptr<BodyStream> GetBodyStream()
    {
      // If m_bodyStream was moved before. nullptr is returned
      return std::move(this->m_bodyStream);
    }

    /**
     * @brief Get HTTP response body as vector of bytes.
     */
    std::vector<uint8_t>& GetBody() { return this->m_body; }

    /**
     * @brief Get HTTP response body as vector of bytes.
     */
    std::vector<uint8_t> const& GetBody() const { return this->m_body; }
  };

}}} // namespace Azure::Core::Http

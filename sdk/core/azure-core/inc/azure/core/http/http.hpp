// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP request and response functionality.
 */

#pragma once

#include "azure/core/http/body_stream.hpp"
#include "azure/core/internal/contract.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef TESTING_BUILD
// Define the class used from tests to validate retry enabled
namespace Azure { namespace Core { namespace Test {
  class TestHttp_getters_Test;
  class TestHttp_query_parameter_Test;
}}} // namespace Azure::Core::Test
#endif

namespace Azure { namespace Core { namespace Http {

  namespace Details {
    // returns left map plus all items in right
    // when duplicates, left items are preferred
    static std::map<std::string, std::string> MergeMaps(
        std::map<std::string, std::string> left,
        std::map<std::string, std::string> const& right)
    {
      left.insert(right.begin(), right.end());
      return left;
    }
  } // namespace Details

  /**
   * @brief HTTP transport implementation used.
   */
  enum class TransportKind
  {
    // TODO move this to Factory
    Curl, ///< CURL.
    WinHttp ///< WinHTTP.
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
   * @param method A value of #HttpMethod value.
   *
   * @return String name that corresponds to a value of #HttpMethod type.
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

  // Url represent the location where a request will be performed. It can be parsed and init from
  // a string that contains all Url parts (scheme, host, path, etc).
  // Authority is not currently supported.
  /**
   * @brief URL.
   */
  class Url {
    // Let Request class to be able to set retry enabled ON
    friend class Request;

  private:
    std::string m_scheme;
    std::string m_host;
    int m_port{-1};
    std::string m_encodedPath;
    std::string m_fragment;
    // query parameters are all decoded
    std::map<std::string, std::string> m_queryParameters;
    std::map<std::string, std::string> m_retryQueryParameters;
    bool m_retryModeEnabled{false};

    /*********  private static methods for all instances *******/
    static std::string Decode(const std::string& value);
    static std::string EncodeHost(const std::string& host);
    static std::string EncodePath(const std::string& path);
    static std::string EncodeQuery(const std::string& query);
    static std::string EncodeFragment(const std::string& fragment);
    static std::string EncodeImpl(
        const std::string& source,
        const std::function<bool(int)>& should_encode);

    void StartRetry()
    {
      m_retryModeEnabled = true;
      m_retryQueryParameters.clear();
    }

    void StopRetry()
    {
      m_retryModeEnabled = false;
      m_retryQueryParameters.clear();
    }

  public:
    /**
     * @brief Construct an empty URL.
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
     * @param isHostEncoded `true` if \p host is URL-encoded, `false` otherwise.
     */
    void SetHost(const std::string& host, bool isHostEncoded = false)
    {
      m_host = isHostEncoded ? host : EncodeHost(host);
    }

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
     * @param isPathEncoded `true` if \p fragment is URL-encoded, `false` otherwise.
     */
    void SetPath(const std::string& path, bool isPathEncoded = false)
    {
      m_encodedPath = isPathEncoded ? path : EncodePath(path);
    }

    /**
     * @brief Set URL fragment.
     *
     * @param fragment URL fragment.
     * @param isFragmentEncoded `true` if \p fragment is URL-encoded, `false` otherwise.
     */
    void SetFragment(const std::string& fragment, bool isFragmentEncoded = false)
    {
      m_fragment = isFragmentEncoded ? fragment : EncodeFragment(fragment);
    }

    // ===== APIs for mutating URL state: ======

    /**
     * @brief Append an element of URL path.
     *
     * @param path URL path element to append.
     * @param isPathEncoded `true` if \p path is URL-encoded, `false` otherwise.
     */
    void AppendPath(const std::string& path, bool isPathEncoded = false)
    {
      if (!m_encodedPath.empty() && m_encodedPath.back() != '/')
      {
        m_encodedPath += '/';
      }
      m_encodedPath += isPathEncoded ? path : EncodePath(path);
    }

    /**
     * @brief Append an HTTP query parameter.
     *
     * @note Overrides previous query parameters.
     * @remark A query key can't contain any characters that need to be URL-encoded (per RFC).
     *
     * @param key HTTP query parameter.
     * @param value HTTP quary parameter value.
     * @param isValueEncoded `true` if \p value is URL-encoded, `false` otherwise.
     */
    void AppendQuery(const std::string& key, const std::string& value, bool isValueEncoded = false)
    {
      std::string encoded_value = isValueEncoded ? Decode(value) : value;
      if (m_retryModeEnabled)
      {
        m_retryQueryParameters[key] = encoded_value;
      }
      else
      {
        m_queryParameters[key] = encoded_value;
      }
    }

    /**
     * @brief Append a HTTP query.
     *
     * @note All the required HTTP query parts should be URL-encoded.
     *
     * @param encodedQueries HTTP query.
     */
    void AppendQueries(const std::string& encodedQueries);

    /**
     * @brief Removes a HTTP query parameter.
     *
     * @param key HTTP query parameter to remove.
     */
    void RemoveQuery(const std::string& key)
    {
      m_queryParameters.erase(key);
      m_retryQueryParameters.erase(key);
    }

    /************** API to read values from Url ***************/
    /**
     * @brief Get URL host.
     */
    std::string GetHost() const { return m_host; }

    /**
     * @brief Get URL path.
     */
    const std::string& GetPath() const { return m_encodedPath; }

    /**
     * @brief Get all the query paramters in the URL.
     *
     * @note Retry parameters have preference and will override any value from the initial query
     * parameters.
     * @note All the values are URL-encoded.
     *
     * @return URL query parameters.
     */
    const std::map<std::string, std::string> GetQuery() const
    {
      return Details::MergeMaps(m_retryQueryParameters, m_queryParameters);
    }

    /**
     * @brief Get relative URL.
     */
    std::string GetRelativeUrl() const;

    /**
     * @brief Get relative URL.
     * @remark All the query parameters are URL-encoded.
     */
    std::string GetAbsoluteUrl() const;
  };

  /**
   * @brief HTTP request.
   */
  class Request {
    friend class RetryPolicy;
#ifdef TESTING_BUILD
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

    void StartRetry(); // only called by retry policy
    void StopRetry(); // only called by retry policy

  public:
    /**
     * @brief Construct an HTTP request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream HTTP #BodyStream.
     * @param downloadViaStream
     */
    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream, bool downloadViaStream)
        : m_method(std::move(httpMethod)), m_url(std::move(url)), m_bodyStream(bodyStream),
          m_retryModeEnabled(false), m_isDownloadViaStream(downloadViaStream)
    {
    }

    /**
     * @brief Construct an HTTP request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param bodyStream HTTP #BodyStream.
     */
    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream)
        : Request(httpMethod, std::move(url), bodyStream, false)
    {
    }

    /**
     * @brief Construct an HTTP request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     * @param downloadViaStream
     */
    // Typically used for GET with no request body that can return bodyStream
    explicit Request(HttpMethod httpMethod, Url url, bool downloadViaStream)
        : Request(
            httpMethod,
            std::move(url),
            NullBodyStream::GetNullBodyStream(),
            downloadViaStream)
    {
    }

    /**
     * @brief Construct an HTTP request.
     *
     * @param httpMethod HTTP method.
     * @param url URL.
     */
    // Typically used for GET with no request body.
    explicit Request(HttpMethod httpMethod, Url url)
        : Request(httpMethod, std::move(url), NullBodyStream::GetNullBodyStream(), false)
    {
    }

    /**
     * @brief Add an HTTP header.
     *
     * @param name HTTP header name.
     * @param value HTTP header value.
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
     * @brief Get HTTP body as #BodyStream.
     */
    BodyStream* GetBodyStream() { return this->m_bodyStream; }

    /**
     * @brief Get HTTP message prior to HTTP body.
     */
    std::string GetHTTPMessagePreBody() const;

    /**
     * @brief Get upload chunk size.
     */
    int64_t GetUploadChunkSize() { return this->m_uploadChunkSize; }

    /**
     * @brief
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

  /*
   * RawResponse exceptions
   */
  /**
   * @brief Couldn't resolve HTTP host.
   */
  struct CouldNotResolveHostException : public std::runtime_error
  {
    explicit CouldNotResolveHostException(std::string const& msg) : std::runtime_error(msg) {}
  };

  // Any other exception from transport layer without an specific exception defined above
  /**
   * @brief HTTP transport layer error.
   */
  struct TransportException : public std::runtime_error
  {
    explicit TransportException(std::string const& msg) : std::runtime_error(msg) {}
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

    // ===== Methods used to build HTTP response =====

    /**
     * @brief Add header to the HTTP response.
     *
     * @param name HTTP response header name.
     * @param value HTTP response header value.
     */
    void AddHeader(std::string const& name, std::string const& value);

    /**
     * @brief Add header to the HTTP response.
     *
     * @param header HTTP response header in RFCnnnn format (`OWS header-value OWS`).
     */
    // rfc form header-name: OWS header-value OWS
    void AddHeader(std::string const& header);

    /**
     * @brief Add header to the HTTP response.
     * @detail HTTP response header should be in RFCnnnn format (`OWS header-value OWS`).
     *
     * @param begin Pointer to the first byte of the header string in RFCnnnn format.
     * @param last Pointer to the last byte of the header string in RFCnnnn format.
     */
    void AddHeader(uint8_t const* const begin, uint8_t const* const last);

    /**
     * @brief Set #BodyStream for this HTTP response.
     *
     * @param stream HTTP #BodyStream.
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
     * @brief Get HTTP response body as #BodyStream.
     */
    std::unique_ptr<BodyStream> GetBodyStream()
    {
      // If m_bodyStream was moved before. nullpr is returned
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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

// Define the class used from tests to validate retry enabled
namespace Azure { namespace Core { namespace Test {
  class TestHttp_getters_Test;
  class TestHttp_query_parameter_Test;
}}} // namespace Azure::Core::Test

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

  enum class TransportKind
  {
    // TODO move this to Factory
    Curl,
    WinHttp
  };

  enum class HttpStatusCode
  {
    None = 0,

    // 1xx (information) Status Codes:
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102,
    EarlyHints = 103,

    // 2xx (successful) Status Codes:
    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207,
    AlreadyReported = 208,
    ImUsed = 226,

    // 3xx (redirection) Status Codes:
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    // 4xx (client error) Status Codes:
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    PayloadTooLarge = 413,
    UriTooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    MisdirectedRequest = 421,
    UnprocessableEntity = 422,
    Locked = 423,
    FailedDependency = 424,
    TooEarly = 425,
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,

    // 5xx (server error) Status Codes:
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507,
    LoopDetected = 508,
    NotExtended = 510,
    NetworkAuthenticationRequired = 511,
  };

  enum class HttpMethod
  {
    Get,
    Head,
    Post,
    Put,
    Delete,
    Patch,
  };

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

  enum class BodyType
  {
    Buffer,
    Stream,
  };

  // Url represent the location where a request will be performed. It can be parsed and init from an
  // a string that contains all Url parts (scheme, host, path, etc).
  // Authority is not currently supported.
  class Url {
    // Let Request class to be able to set retry enabled ON
    friend class Request;

  private:
    std::string m_scheme;
    std::string m_host;
    int m_port{-1};
    // m_path is encoded
    std::string m_path;
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

    std::string GetEncodedQuery() const;

  public:
    // Create empty Url instance. Usually for building Url from scratch
    Url() {}

    // Create Url from an url-encoded string. Usually from pre-built url with query parameters (like
    // SaS) url is expected to be already url-encoded.
    explicit Url(const std::string& url);

    /************* Builder Url functions ****************/
    /******** API for building Url from scratch. Override state ********/

    void SetScheme(const std::string& scheme) { m_scheme = scheme; }

    void SetHost(const std::string& host, bool isHostEncoded = false)
    {
      m_host = isHostEncoded ? host : EncodeHost(host);
    }

    void SetPort(uint16_t port) { m_port = port; }

    void SetPath(const std::string& path, bool isPathEncoded = false)
    {
      m_path = isPathEncoded ? path : EncodePath(path);
    }

    void SetFragment(const std::string& fragment, bool isFragmentEncoded = false)
    {
      m_fragment = isFragmentEncoded ? fragment : EncodeFragment(fragment);
    }

    /******** API for mutating Url state ********/

    // Path is mostly expected to be appended without url-encoding. Be default, path will be encoded
    // before it is added to Url. \p isPathEncoded can set to true to avoid encoding.
    void AppendPath(const std::string& path, bool isPathEncoded = false)
    {
      if (!m_path.empty() && m_path.back() != '/')
      {
        m_path += '/';
      }
      m_path += isPathEncoded ? path : EncodePath(path);
    }

    // the value from query parameter is mostly expected to be non-url-encoded and it will be
    // encoded before adding to url by default. Use \p isValueEncoded = true when the value is
    // already encoded.
    //
    // Note: a query key can't contain any chars that needs to be url-encoded. (by RFC).
    //
    // Note: AppendQuery override previous query parameters.
    void AppendQuery(const std::string& key, const std::string& value, bool isValueEncoded = false)
    {
      if (m_retryModeEnabled)
      {
        m_retryQueryParameters[key] = isValueEncoded ? Decode(value) : value;
      }
      else
      {
        m_queryParameters[key] = isValueEncoded ? Decode(value) : value;
      }
    }

    // query must be encoded.
    void AppendQueries(const std::string& query);

    // removes a query parameter
    void RemoveQuery(const std::string& key)
    {
      m_queryParameters.erase(key);
      m_retryQueryParameters.erase(key);
    }

    /************** API to read values from Url ***************/

    std::string GetHost() const { return m_host; }

    const std::string& GetPath() const { return m_path; }

    // Copy from query parameters list. Query parameters from retry map have preference and will
    // override any value from the initial query parameters from the request
    //
    // Note: Query values added with url-encoding will be encoded in the list. No decoding is done
    // on values.
    const std::map<std::string, std::string> GetQuery() const
    {
      return Details::MergeMaps(m_retryQueryParameters, m_queryParameters);
    }

    std::string GetRelativeUrl() const;

    // Url with encoded query parameters
    std::string GetAbsoluteUrl() const;
  };

  class Request {
    friend class RetryPolicy;
    // make tests classes friends to validate set Retry
    friend class Azure::Core::Test::TestHttp_getters_Test;
    friend class Azure::Core::Test::TestHttp_query_parameter_Test;

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
    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream, bool downloadViaStream)
        : m_method(std::move(httpMethod)), m_url(std::move(url)), m_bodyStream(bodyStream),
          m_retryModeEnabled(false), m_isDownloadViaStream(downloadViaStream)
    {
    }

    explicit Request(HttpMethod httpMethod, Url url, BodyStream* bodyStream)
        : Request(httpMethod, std::move(url), bodyStream, false)
    {
    }

    // Typically used for GET with no request body that can return bodyStream
    explicit Request(HttpMethod httpMethod, Url url, bool downloadViaStream)
        : Request(
            httpMethod,
            std::move(url),
            NullBodyStream::GetNullBodyStream(),
            downloadViaStream)
    {
    }

    // Typically used for GET with no request body.
    explicit Request(HttpMethod httpMethod, Url url)
        : Request(httpMethod, std::move(url), NullBodyStream::GetNullBodyStream(), false)
    {
    }

    void AddHeader(std::string const& name, std::string const& value);
    void RemoveHeader(std::string const& name);
    void SetUploadChunkSize(int64_t size) { this->m_uploadChunkSize = size; }

    // Methods used by transport layer (and logger) to send request
    HttpMethod GetMethod() const;
    std::map<std::string, std::string> GetHeaders() const;
    BodyStream* GetBodyStream() { return this->m_bodyStream; }
    std::string GetHTTPMessagePreBody() const;
    int64_t GetUploadChunkSize() { return this->m_uploadChunkSize; }
    bool IsDownloadViaStream() { return this->m_isDownloadViaStream; }
    Url& GetUrl() { return this->m_url; }
    Url const& GetUrl() const { return this->m_url; }
  };

  /*
   * RawResponse exceptions
   */
  struct CouldNotResolveHostException : public std::runtime_error
  {
    explicit CouldNotResolveHostException(std::string const& msg) : std::runtime_error(msg) {}
  };

  // Any other excpetion from transport layer without an specific exception defined above
  struct TransportException : public std::runtime_error
  {
    explicit TransportException(std::string const& msg) : std::runtime_error(msg) {}
  };

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
    explicit RawResponse(
        int32_t majorVersion,
        int32_t minorVersion,
        HttpStatusCode statusCode,
        std::string const& reasonPhrase)
        : RawResponse(majorVersion, minorVersion, statusCode, reasonPhrase, nullptr)
    {
    }

    // Methods used to build HTTP response
    void AddHeader(std::string const& name, std::string const& value);
    // rfc form header-name: OWS header-value OWS
    void AddHeader(std::string const& header);
    void AddHeader(uint8_t const* const begin, uint8_t const* const last);
    void SetBodyStream(std::unique_ptr<BodyStream> stream);
    void SetBody(std::vector<uint8_t> body) { this->m_body = std::move(body); }

    // adding getters for version and stream body. Clang will complain on Mac if we have unused
    // fields in a class
    int32_t GetMajorVersion() const { return this->m_majorVersion; }
    int32_t GetMinorVersion() const { return this->m_minorVersion; }
    HttpStatusCode GetStatusCode() const;
    std::string const& GetReasonPhrase() const;
    std::map<std::string, std::string> const& GetHeaders() const;
    std::unique_ptr<BodyStream> GetBodyStream()
    {
      // If m_bodyStream was moved before. nullpr is returned
      return std::move(this->m_bodyStream);
    }
    std::vector<uint8_t>& GetBody() { return this->m_body; }
    std::vector<uint8_t> const& GetBody() const { return this->m_body; }
  };

}}} // namespace Azure::Core::Http

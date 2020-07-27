// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "body_stream.hpp"

#include <algorithm>
#include <internal/contract.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Http {

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

  // parses full url into protocol, host, port, path and query.
  // Authority is not currently supported.
  class URL {
  private:
    std::string m_scheme;
    std::string m_host;
    std::string m_port;
    std::string m_path;
    std::map<std::string, std::string> m_queryParameters;

    /**
     * Will check if there are any query parameter in url looking for symbol '?'
     * If it is found, it will insert query parameters to m_queryParameters internal field
     * and remove it from url
     */
    const std::string SaveAndRemoveQueryParameter(std::string const& url)
    {

      const auto firstPosition = std::find(url.begin(), url.end(), '?');
      if (firstPosition == url.end())
      {
        return url; // not query parameters
      }

      auto position = firstPosition; // position of symbol ?
      while (position != url.end())
      {
        ++position; // skip over the ? or &
        const auto nextPosition = std::find(position, url.end(), '&');
        const auto equalChar = std::find(position, nextPosition, '=');
        auto valueStart = equalChar;
        if (valueStart != nextPosition)
        {
          ++valueStart; // skip = symbol
        }

        // Note: if there is another = symbol before nextPosition, it will be part of the
        // paramenter value. And if there is not a ? symbol, we add empty string as value
        this->m_queryParameters.insert(std::pair<std::string, std::string>(
            std::string(position, equalChar), std::string(valueStart, nextPosition)));

        position = nextPosition;
      }

      return std::string(url.begin(), firstPosition);
    }

  public:
    URL(std::string const& url);
    void AppendPath(std::string const& path)
    {
      // Constructor makes sure path never keeps slash at the end so we can feel OK on adding slash
      // on every append path
      this->m_path += "/" + path;
    }
    std::string ToString() const
    {
      auto port = this->m_port.size() > 0 ? ":" + this->m_port : "";
      return this->m_scheme + "://" + this->m_host + port + this->m_path;
    }
    std::string GetPath() const { return this->m_path; }
    std::string GetHost() const { return this->m_host; }
    std::map<std::string, std::string> GetQueryParameters() const
    {
      return this->m_queryParameters;
    }
    void AddQueryParameter(std::string const& name, std::string const& value)
    {
      this->m_queryParameters.insert(std::pair<std::string, std::string>(name, value));
    }
  };

  class Request {

  private:
    HttpMethod m_method;
    URL m_url;
    std::map<std::string, std::string> m_headers;
    std::map<std::string, std::string> m_retryHeaders;
    std::map<std::string, std::string> m_retryQueryParameters;

    BodyStream* m_bodyStream;

    // flag to know where to insert header
    bool m_retryModeEnabled;

    // returns left map plus all items in right
    // when duplicates, left items are preferred
    static std::map<std::string, std::string> MergeMaps(
        std::map<std::string, std::string> left,
        std::map<std::string, std::string> const& right)
    {
      left.insert(right.begin(), right.end());
      return left;
    }

    std::string GetQueryString() const;

    // This value can be used to override the default value that an http transport adapter uses to
    // read and upload chunks of data from the payload body stream. If it is not set, the transport
    // adapter will decide chunk size.
    int64_t m_uploadChunkSize = 0;

  public:
    explicit Request(HttpMethod httpMethod, std::string const& url, BodyStream* bodyStream)
        : m_method(std::move(httpMethod)), m_url(url), m_bodyStream(bodyStream),
          m_retryModeEnabled(false)
    {
    }

    // Typically used for GET with no request body.
    explicit Request(HttpMethod httpMethod, std::string const& url)
        : Request(httpMethod, url, NullBodyStream::GetNullBodyStream())
    {
    }

    // Methods used to build HTTP request
    void AppendPath(std::string const& path);
    void AddQueryParameter(std::string const& name, std::string const& value);
    void AddHeader(std::string const& name, std::string const& value);
    void StartRetry(); // only called by retry policy
    void SetUploadChunkSize(int64_t size) { this->m_uploadChunkSize = size; }

    // Methods used by transport layer (and logger) to send request
    HttpMethod GetMethod() const;
    std::string GetEncodedUrl() const; // should call URL encode
    std::string GetHost() const;
    std::map<std::string, std::string> GetHeaders() const;
    BodyStream* GetBodyStream() { return this->m_bodyStream; }
    std::string GetHTTPMessagePreBody() const;
    int64_t GetUploadChunkSize() { return this->m_uploadChunkSize; }
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

    // adding getters for version and stream body. Clang will complain on Mac if we have unused
    // fields in a class
    int32_t GetMajorVersion() const { return this->m_majorVersion; }
    int32_t GetMinorVersion() const { return this->m_minorVersion; }
    HttpStatusCode GetStatusCode() const;
    std::string const& GetReasonPhrase();
    std::map<std::string, std::string> const& GetHeaders() const;
    std::unique_ptr<BodyStream> GetBodyStream()
    {
      // If m_bodyStream was moved before. nullpr is returned
      return std::move(this->m_bodyStream);
    }
  };

}}} // namespace Azure::Core::Http

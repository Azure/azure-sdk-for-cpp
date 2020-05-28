// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <internal/contract.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  // BodyStream is used to read data to/from a service
  class BodyStream {
  public:
    static BodyStream* null;

    // Returns the length of the data; used with the HTTP Content-Length header
    virtual uint64_t Length() = 0;

    // Resets the stream back to the beginning (for retries)
    // Derived classes that send data in an HTTP request MUST override this and implement it
    // properly.
    virtual void Rewind()
    {
      throw "Not Implemented"; // TODO: Replace with best practice as defined by guideline
    };

    // Reads more data; EOF if return < count; throws if error/canceled
    virtual uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t offset, uint64_t count)
        = 0;

    // Closes the stream; typically called after all data read or if an error occurs.
    virtual void Close() = 0;
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

  class Request {

  private:
    // query needs to be first or at least before url, since url might update it
    std::map<std::string, std::string> m_queryParameters;

    HttpMethod _method;
    std::string _url;
    std::map<std::string, std::string> m_headers;
    std::map<std::string, std::string> m_retryHeaders;
    std::map<std::string, std::string> m_retryQueryParameters;
    // Request can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
    BodyStream* m_bodyStream;
    std::vector<uint8_t> m_bodyBuffer;

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

    /**
     * Will check if there are any query parameter in url looking for symbol '?'
     * If it is found, it will insert query parameters to m_queryParameters internal field
     * and remove it from url
     */
    const std::string parseUrl(std::string const& url)
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
        m_queryParameters.insert(std::pair<std::string, std::string>(
            std::string(position, equalChar), std::string(valueStart, nextPosition)));

        position = nextPosition;
      }

      return std::string(url.begin(), firstPosition);
    }

    Request(
        HttpMethod httpMethod,
        std::string const& url,
        BodyStream* bodyStream,
        std::vector<uint8_t> bodyBuffer)
        : _method(std::move(httpMethod)), _url(parseUrl(url)), m_bodyStream(bodyStream),
          m_bodyBuffer(std::move(bodyBuffer)), m_retryModeEnabled(false)
    {
      // TODO: parse url
    }

  public:
    Request(HttpMethod httpMethod, std::string const& url)
        : Request(httpMethod, url, BodyStream::null, std::vector<uint8_t>())
    {
    }

    Request(HttpMethod httpMethod, std::string const& url, std::vector<uint8_t> const& bodyBuffer)
        : Request(httpMethod, url, BodyStream::null, bodyBuffer)
    {
    }

    Request(HttpMethod httpMethod, std::string const& url, BodyStream* bodyStream)
        : Request(httpMethod, url, bodyStream, std::vector<uint8_t>())
    {
    }

    // Methods used to build HTTP request
    void AddPath(std::string const& path);
    void AddQueryParameter(std::string const& name, std::string const& value);
    void AddHeader(std::string const& name, std::string const& value);
    void StartRetry(); // only called by retry policy

    // Methods used by transport layer (and logger) to send request
    HttpMethod GetMethod();
    std::string GetEncodedUrl(); // should return encoded url
    std::map<std::string, std::string> GetHeaders();
    BodyStream* GetBodyStream();
    std::vector<uint8_t> const& GetBodyBuffer();
  };

  /*
   * Response exceptions
   */
  struct CouldNotResolveHostException : public std::exception
  {
    const char* what() const throw() { return "couldnt resolve host"; }
  };

  struct ErrorWhileWrittingResponse : public std::exception
  {
    const char* what() const throw() { return "couldnt write response"; }
  };

  // Any other excpetion from transport layer without an specific exception defined above
  struct TransportException : public std::exception
  {
    const char* what() const throw() { return "Error on transport layer while sending request"; }
  };

  class Response {

  private:
    uint16_t m_majorVersion;
    uint16_t m_minorVersion;
    HttpStatusCode m_statusCode;
    std::string m_reasonPhrase;
    std::map<std::string, std::string> m_headers;

    // Response can contain no body, or either of next bodies (m_bodyBuffer or
    // bodyStream)
    std::vector<uint8_t> m_bodyBuffer;
    Http::BodyStream* m_bodyStream;

    Response(
        int16_t majorVersion,
        uint16_t minorVersion,
        HttpStatusCode statusCode,
        std::string const& reasonPhrase,
        std::vector<uint8_t> const& bodyBuffer,
        BodyStream* const BodyStream)
        : m_majorVersion(majorVersion), m_minorVersion(minorVersion), m_statusCode(statusCode),
          m_reasonPhrase(reasonPhrase), m_bodyBuffer(bodyBuffer), m_bodyStream(BodyStream)
    {
    }

  public:
    Response(
        uint16_t majorVersion,
        uint16_t minorVersion,
        HttpStatusCode statusCode,
        std::string const& reasonPhrase)
        : Response(
              majorVersion,
              minorVersion,
              statusCode,
              reasonPhrase,
              std::vector<uint8_t>(),
              Http::BodyStream::null)
    {
    }

    // Methods used to build HTTP response
    void AddHeader(std::string const& name, std::string const& value);
    void AppendBody(uint8_t* ptr, uint64_t size);

    // Util methods for customers to read response/parse an http response
    HttpStatusCode GetStatusCode();
    std::string const& GetReasonPhrase();
    std::map<std::string, std::string> const& GetHeaders();
    std::vector<uint8_t>& GetBodyBuffer();

    // adding getters for version and stream body. Clang will complain on Mac if we have unused
    // fields in a class
    uint16_t GetmajorVersion() { return m_majorVersion; }
    uint16_t GetMinorVersion() { return m_minorVersion; }
    Http::BodyStream* GetBodyStream() { return m_bodyStream; }
  };

  class Client {
  public:
    static std::unique_ptr<Response> Send(Request& request);
  };
}}} // namespace Azure::Core::Http

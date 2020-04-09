// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <internal/contract.hpp>

namespace azure
{
namespace core
{
namespace http
{

// BodyStream is used to read data to/from a service
class BodyStream
{
public:
  static BodyStream* null;

  // Returns the length of the data; used with the HTTP Content-Length header
  virtual uint64_t Length() = 0;

  // Resets the stream back to the beginning (for retries)
  // Derived classes that send data in an HTTP request MUST override this and implement it properly.
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

class BodyBuffer
{

public:
  static BodyBuffer* null;

  uint8_t const* _bodyBuffer;
  uint64_t _bodyBufferSize;
  BodyBuffer(uint8_t const* bodyBuffer, uint64_t bodyBufferSize)
      : _bodyBuffer(bodyBuffer), _bodyBufferSize(bodyBufferSize)
  {
  }
};

enum class HttpMethod
{
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  PATCH,
};

enum class HttpStatusCode
{
  AZ_HTTP_STATUS_CODE_NONE = 0,

  // 1xx (information) Status Codes:
  AZ_HTTP_STATUS_CODE_CONTINUE = 100,
  AZ_HTTP_STATUS_CODE_SWITCHING_PROTOCOLS = 101,
  AZ_HTTP_STATUS_CODE_PROCESSING = 102,
  AZ_HTTP_STATUS_CODE_EARLY_HINTS = 103,

  // 2xx (successful) Status Codes:
  AZ_HTTP_STATUS_CODE_OK = 200,
  AZ_HTTP_STATUS_CODE_CREATED = 201,
  AZ_HTTP_STATUS_CODE_ACCEPTED = 202,
  AZ_HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
  AZ_HTTP_STATUS_CODE_NO_CONTENT = 204,
  AZ_HTTP_STATUS_CODE_RESET_CONTENT = 205,
  AZ_HTTP_STATUS_CODE_PARTIAL_CONTENT = 206,
  AZ_HTTP_STATUS_CODE_MULTI_STATUS = 207,
  AZ_HTTP_STATUS_CODE_ALREADY_REPORTED = 208,
  AZ_HTTP_STATUS_CODE_IM_USED = 226,

  // 3xx (redirection) Status Codes:
  AZ_HTTP_STATUS_CODE_MULTIPLE_CHOICES = 300,
  AZ_HTTP_STATUS_CODE_MOVED_PERMANENTLY = 301,
  AZ_HTTP_STATUS_CODE_FOUND = 302,
  AZ_HTTP_STATUS_CODE_SEE_OTHER = 303,
  AZ_HTTP_STATUS_CODE_NOT_MODIFIED = 304,
  AZ_HTTP_STATUS_CODE_USE_PROXY = 305,
  AZ_HTTP_STATUS_CODE_TEMPORARY_REDIRECT = 307,
  AZ_HTTP_STATUS_CODE_PERMANENT_REDIRECT = 308,

  // 4xx (client error) Status Codes:
  AZ_HTTP_STATUS_CODE_BAD_REQUEST = 400,
  AZ_HTTP_STATUS_CODE_UNAUTHORIZED = 401,
  AZ_HTTP_STATUS_CODE_PAYMENT_REQUIRED = 402,
  AZ_HTTP_STATUS_CODE_FORBIDDEN = 403,
  AZ_HTTP_STATUS_CODE_NOT_FOUND = 404,
  AZ_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED = 405,
  AZ_HTTP_STATUS_CODE_NOT_ACCEPTABLE = 406,
  AZ_HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
  AZ_HTTP_STATUS_CODE_REQUEST_TIMEOUT = 408,
  AZ_HTTP_STATUS_CODE_CONFLICT = 409,
  AZ_HTTP_STATUS_CODE_GONE = 410,
  AZ_HTTP_STATUS_CODE_LENGTH_REQUIRED = 411,
  AZ_HTTP_STATUS_CODE_PRECONDITION_FAILED = 412,
  AZ_HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE = 413,
  AZ_HTTP_STATUS_CODE_URI_TOO_LONG = 414,
  AZ_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
  AZ_HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE = 416,
  AZ_HTTP_STATUS_CODE_EXPECTATION_FAILED = 417,
  AZ_HTTP_STATUS_CODE_MISDIRECTED_REQUEST = 421,
  AZ_HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY = 422,
  AZ_HTTP_STATUS_CODE_LOCKED = 423,
  AZ_HTTP_STATUS_CODE_FAILED_DEPENDENCY = 424,
  AZ_HTTP_STATUS_CODE_TOO_EARLY = 425,
  AZ_HTTP_STATUS_CODE_UPGRADE_REQUIRED = 426,
  AZ_HTTP_STATUS_CODE_PRECONDITION_REQUIRED = 428,
  AZ_HTTP_STATUS_CODE_TOO_MANY_REQUESTS = 429,
  AZ_HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  AZ_HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

  // 5xx (server error) Status Codes:
  AZ_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
  AZ_HTTP_STATUS_CODE_NOT_IMPLEMENTED = 501,
  AZ_HTTP_STATUS_CODE_BAD_GATEWAY = 502,
  AZ_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE = 503,
  AZ_HTTP_STATUS_CODE_GATEWAY_TIMEOUT = 504,
  AZ_HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
  AZ_HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES = 506,
  AZ_HTTP_STATUS_CODE_INSUFFICIENT_STORAGE = 507,
  AZ_HTTP_STATUS_CODE_LOOP_DETECTED = 508,
  AZ_HTTP_STATUS_CODE_NOT_EXTENDED = 510,
  AZ_HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511,
};

class Request
{

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
  BodyBuffer* m_bodyBuffer;

  // flag to know where to insert header
  bool m_retryModeEnabled;

  // returns left map plus all items in right
  // when duplicates, left items are preferred
  static std::map<std::string, std::string> mergeMaps(
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

      // Note: if there is another = symbol before nextPosition, it will be part of the paramenter
      // value. And if there is not a ? symbol, we add empty string as value
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
      BodyBuffer* bodyBuffer)
      : _method(std::move(httpMethod)), _url(parseUrl(url)), m_bodyStream(bodyStream),
        m_bodyBuffer(bodyBuffer), m_retryModeEnabled(false)
  {
    // TODO: parse url
  }

public:
  Request(HttpMethod httpMethod, std::string const& url)
      : Request(httpMethod, url, BodyStream::null, BodyBuffer::null)
  {
  }

  Request(HttpMethod httpMethod, std::string const& url, BodyBuffer* bodyBuffer)
      : Request(httpMethod, url, BodyStream::null, bodyBuffer)
  {
  }

  Request(HttpMethod httpMethod, std::string const& url, BodyStream* bodyStream)
      : Request(httpMethod, url, bodyStream, BodyBuffer::null)
  {
  }

  // Methods used to build HTTP request
  void addPath(std::string const& path);
  void addQueryParameter(std::string const& name, std::string const& value);
  void addHeader(std::string const& name, std::string const& value);
  void startRetry(); // only called by retry policy

  // Methods used by transport layer (and logger) to send request
  HttpMethod getMethod();
  std::string getEncodedUrl(); // should return encoded url
  std::map<std::string, std::string> getHeaders();
  BodyStream* getBodyStream();
  BodyBuffer* getBodyBuffer();
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

class Response
{

private:
  uint16_t m_mayorVersion;
  uint16_t m_minorVersion;
  HttpStatusCode m_statusCode;
  std::string m_reasonPhrase;
  std::map<std::string, std::string> m_headers;

  // Response can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  std::vector<uint8_t> m_bodyBuffer;
  http::BodyStream* m_bodyStream;

  Response(
      uint16_t mayorVersion,
      uint16_t minorVersion,
      HttpStatusCode statusCode,
      std::string const& reasonPhrase,
      std::vector<uint8_t> const& bodyBuffer,
      BodyStream* const BodyStream)
      : m_mayorVersion(mayorVersion), m_minorVersion(minorVersion), m_statusCode(statusCode),
        m_reasonPhrase(reasonPhrase), m_bodyBuffer(bodyBuffer), m_bodyStream(BodyStream)
  {
  }

public:
  Response(
      uint16_t mayorVersion,
      uint16_t minorVersion,
      HttpStatusCode statusCode,
      std::string const& reasonPhrase)
      : Response(
            mayorVersion,
            minorVersion,
            statusCode,
            reasonPhrase,
            std::vector<uint8_t>(),
            http::BodyStream::null)
  {
  }

  // Methods used by transport layer upon receiving a response
  void AddHeader(std::string const& name, std::string const& value);
  void AppendBody(uint8_t* ptr, uint64_t size);

  // Util methods for customers to read response/parse an http response
  HttpStatusCode GetStatusCode();
  std::string const& GetReasonPhrase();
  std::map<std::string, std::string> const& GetHeaders();
  std::vector<uint8_t>& GetBodyBuffer();

  // adding getters for version and stream body. Clang will complain on Mac if we have unused fields
  // in a class
  uint16_t GetMayorVersion() { return m_mayorVersion; }
  uint16_t GetMinorVersion() { return m_minorVersion; }
  http::BodyStream* GetBodyStream() { return m_bodyStream; }
};

class Client
{
public:
  static Response* Send(Request& request);
};

} // namespace http
} // namespace core
} // namespace azure

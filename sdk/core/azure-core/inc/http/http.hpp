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
  NONE = 0,

  // 1xx (information) Status Codes:
  CONTINUE = 100,
  SWITCHING_PROTOCOLS = 101,
  PROCESSING = 102,
  EARLY_HINTS = 103,

  // 2xx (successful) Status Codes:
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NON_AUTHORITATIVE_INFORMATION = 203,
  NO_CONTENT = 204,
  RESET_CONTENT = 205,
  PARTIAL_CONTENT = 206,
  MULTI_STATUS = 207,
  ALREADY_REPORTED = 208,
  IM_USED = 226,

  // 3xx (redirection) Status Codes:
  MULTIPLE_CHOICES = 300,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  USE_PROXY = 305,
  TEMPORARY_REDIRECT = 307,
  PERMANENT_REDIRECT = 308,

  // 4xx (client error) Status Codes:
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  PAYMENT_REQUIRED = 402,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  PROXY_AUTHENTICATION_REQUIRED = 407,
  REQUEST_TIMEOUT = 408,
  CONFLICT = 409,
  GONE = 410,
  LENGTH_REQUIRED = 411,
  PRECONDITION_FAILED = 412,
  PAYLOAD_TOO_LARGE = 413,
  URI_TOO_LONG = 414,
  UNSUPPORTED_MEDIA_TYPE = 415,
  RANGE_NOT_SATISFIABLE = 416,
  EXPECTATION_FAILED = 417,
  MISDIRECTED_REQUEST = 421,
  UNPROCESSABLE_ENTITY = 422,
  LOCKED = 423,
  FAILED_DEPENDENCY = 424,
  TOO_EARLY = 425,
  UPGRADE_REQUIRED = 426,
  PRECONDITION_REQUIRED = 428,
  TOO_MANY_REQUESTS = 429,
  REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  UNAVAILABLE_FOR_LEGAL_REASONS = 451,

  // 5xx (server error) Status Codes:
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
  GATEWAY_TIMEOUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  VARIANT_ALSO_NEGOTIATES = 506,
  INSUFFICIENT_STORAGE = 507,
  LOOP_DETECTED = 508,
  NOT_EXTENDED = 510,
  NETWORK_AUTHENTICATION_REQUIRED = 511,
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
  uint16_t m_majorVersion;
  uint16_t m_minorVersion;
  HttpStatusCode m_statusCode;
  std::string m_reasonPhrase;
  std::map<std::string, std::string> m_headers;

  // Response can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  std::vector<uint8_t> m_bodyBuffer;
  http::BodyStream* m_bodyStream;

  Response(
      uint16_t majorVersion,
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
  uint16_t GetmajorVersion() { return m_majorVersion; }
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

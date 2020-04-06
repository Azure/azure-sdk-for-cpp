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

class HttpVersion
{
private:
  uint16_t m_mayorVersion;
  uint16_t m_minorVersion;
  static const std::string m_title;
  static const std::string m_separator;

public:
  HttpVersion(uint16_t mayorVersion, uint16_t minorVersion)
      : m_mayorVersion(mayorVersion), m_minorVersion(minorVersion)
  {
  }

  HttpVersion() : HttpVersion(0, 0) {}

  operator std::string() const
  {
    return m_title + std::to_string(m_mayorVersion) + m_separator + std::to_string(m_minorVersion);
  }
};

class Response
{

private:
  http::HttpVersion m_httpVersion;
  uint16_t m_statusCode;
  std::string m_reasonPhrase;
  std::map<std::string, std::string> m_headers;

  // Response can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  http::BodyBuffer* m_bodyBuffer;
  http::BodyStream* m_bodyStream;

  Response(
      uint16_t statusCode,
      std::string const& reasonPhrase,
      BodyBuffer* const bodyBuffer,
      BodyStream* const BodyStream)
      : m_statusCode(statusCode), m_reasonPhrase(reasonPhrase), m_bodyBuffer(bodyBuffer),
        m_bodyStream(BodyStream)
  {
  }

public:
  Response(uint16_t statusCode, std::string const& reasonPhrase)
      : Response(statusCode, reasonPhrase, http::BodyBuffer::null, http::BodyStream::null)
  {
  }

  // default constructor for bulding from base response
  Response() : Response(0, "", http::BodyBuffer::null, http::BodyStream::null) {}

  // Methods used to build HTTP response
  void setVersion(uint16_t mayorVersion, uint16_t minorVersion)
  {
    m_httpVersion = http::HttpVersion(mayorVersion, minorVersion);
  }
  void setStatusCode(uint16_t statusCode) { m_statusCode = statusCode; }
  void setReasonPhrase(std::string reasonPhrase) { m_reasonPhrase = reasonPhrase; }
  void addHeader(std::string const& name, std::string const& value);
  void setBody(BodyBuffer* bodyBuffer);
  void setBody(BodyStream* bodyStream);

  // Methods used by transport layer (and logger) to send response
  uint16_t getStatusCode();
  std::string const& getReasonPhrase();
  std::string getHttpVersion();
  std::map<std::string, std::string> const& getHeaders();
  http::BodyStream* getBodyStream();
  http::BodyBuffer* getBodyBuffer();
};

class Client
{
public:
  static Response send(Request& request);
};

} // namespace http
} // namespace core
} // namespace azure

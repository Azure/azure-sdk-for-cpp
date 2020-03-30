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
  // Request(Request const&) = delete;
  void operator=(Request const&) = delete;

  // query needs to be first or at least before url, since url might update it
  std::map<std::string, std::string> _queryParameters;

  HttpMethod _method;
  std::string _url;
  std::map<std::string, std::string> _headers;
  std::map<std::string, std::string> _retryHeaders;
  std::map<std::string, std::string> _retryQueryParameters;
  // Request can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  BodyStream* _bodyStream;
  BodyBuffer* _bodyBuffer;

  // flag to know where to insert header
  bool _retryModeEnabled;

  // returns left map plus all items in right
  // when duplicates, left items are preferred
  static std::map<std::string, std::string> mergeMaps(
      std::map<std::string, std::string> const& left,
      std::map<std::string, std::string> const& right)
  {
    auto result = std::map<std::string, std::string>(left);
    std::for_each(right.begin(), right.end(), [&](auto const& pair) { result.insert(pair); });
    return result;
  }

  /**
   * Create an stream from source string. Then use getline and separator to get as many tokens as
   * possible and insert each one to result vector
   */
  static std::vector<std::string> split(std::string source, char separator)
  {
    auto result = std::vector<std::string>();

    auto stringAsStream = std::stringstream(source);
    auto token = std::string();

    while (std::getline(stringAsStream, token, separator))
    {
      result.push_back(token);
    }
    return result;
  }

  /**
   * Will check if there are any query parameter in url looking for symbol '?'
   * If it is found, it will insert query parameters to _queryParameters internal field
   * and remove it from url
   */
  std::string parseUrl(std::string url)
  {
    auto position = url.find('?');

    if (position == std::string::npos)
    {
      return url; // no query parameters. Nothing else to do
    }

    // Get query parameters string and update
    auto queryParameters = url.substr(position + 1);
    url = url.substr(0, position);

    // Split all query parameters (should be separated by &)
    auto queryParametersVector = Request::split(queryParameters, '&');

    // insert each query parameter to internal field
    std::for_each(queryParametersVector.begin(), queryParametersVector.end(), [&](auto query) {
      auto parameter = split(query, '=');
      // Will throw if parameter in query is not valid (i.e. arg:1)
      _queryParameters.insert(std::pair<std::string, std::string>(parameter[0], parameter[1]));
    });

    return url;
  }

  Request(
      HttpMethod httpMethod,
      std::string const& url,
      BodyStream* bodyStream,
      BodyBuffer* bodyBuffer)
      : _method(std::move(httpMethod)), _url(parseUrl(std::move(url))), _bodyStream(bodyStream),
        _bodyBuffer(bodyBuffer), _retryModeEnabled(false)
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

class Response
{

private:
  // Response(Response const&) = delete;
  void operator=(Response const&) = delete;

  uint16_t _statusCode;
  std::string _reasonPhrase;
  std::map<std::string, std::string> _headers;

  // Response can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  http::BodyBuffer* _bodyBuffer;
  http::BodyStream* _bodyStream;

  Response(
      uint16_t statusCode,
      std::string reasonPhrase,
      BodyBuffer* const bodyBuffer,
      BodyStream* const BodyStream)
      : _statusCode(statusCode), _reasonPhrase(reasonPhrase), _bodyBuffer(bodyBuffer),
        _bodyStream(BodyStream)
  {
  }

public:
  Response(uint16_t statusCode, std::string reasonPhrase)
      : Response(statusCode, reasonPhrase, http::BodyBuffer::null, http::BodyStream::null)
  {
  }

  // Methods used to build HTTP response
  void addHeader(std::string const& name, std::string const& value);
  void setBody(BodyBuffer* bodyBuffer);
  void setBody(BodyStream* bodyStream);

  // Methods used by transport layer (and logger) to send response
  uint16_t const& getStatusCode();
  std::string const& getReasonPhrase();
  std::map<std::string, std::string> const& getHeaders();
  http::BodyStream* getBodyStream();
  http::BodyBuffer* getBodyBuffer();
};

class Client
{
public:
  static Response send(Request const& request);
};

} // namespace http
} // namespace core
} // namespace azure

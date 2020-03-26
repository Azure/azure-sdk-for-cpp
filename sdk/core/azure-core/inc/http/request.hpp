// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <map>
#include <string>

#include <internal/contract.hpp>

namespace azure
{
namespace core
{
namespace http
{

class Request
{

private:
  Request(Request const&) = delete;
  void operator=(Request const&) = delete;

  HttpMethod _method;
  std::string _url;
  std::map<std::string, std::string> _headers;
  std::map<std::string, std::string> _retryHeaders;
  std::map<std::string, std::string> _queryParameters;
  std::map<std::string, std::string> _retryQueryParameters;
  // Request can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
  BodyStream& _bodyStream;
  BodyBuffer& _bodyBuffer;

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

  // Will try to inster pair into map. If it fails because key is already there
  // it will override the value for that key
  static void insertOrReplace(
      std::map<std::string, std::string>& map,
      std::pair<std::string, std::string> const& pair)
  {
    auto result = map.insert(pair);
    // if header already exist, override it's value
    if (!result.second)
    {
      result.first->second = pair.second;
    }
  }

  Request(
      HttpMethod httpMethod,
      std::string const& url,
      BodyStream& bodyStream,
      BodyBuffer& bodyBuffer)
      : _method(std::move(httpMethod)), _url(std::move(url)), _bodyStream(bodyStream),
        _bodyBuffer(bodyBuffer), _retryModeEnabled(false)
  {
    // TODO: parse url
  }

public:
  Request(HttpMethod httpMethod, std::string const& url)
      : Request(httpMethod, url, BodyStream::null, BodyBuffer::null)
  {
  }

  Request(HttpMethod httpMethod, std::string const& url, BodyBuffer& bodyBuffer)
      : Request(httpMethod, url, BodyStream::null, bodyBuffer)
  {
  }

  Request(HttpMethod httpMethod, std::string const& url, BodyStream& bodyStream)
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
  BodyStream& getBodyStream();
  BodyBuffer& getBodyBuffer();
};

} // namespace http
} // namespace core
} // namespace azure

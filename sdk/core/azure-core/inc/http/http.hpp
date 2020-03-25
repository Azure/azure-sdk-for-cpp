// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>

#include <internal/contract.hpp>

namespace azure
{
namespace core
{
namespace http
{

namespace http_method
{
enum HttpMethod
{
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  PATCH,
};
}

class Request
{

private:
  http_method::HttpMethod _method;
  std::string _url;
  std::map<std::string, std::string> _headers;
  std::string _body;
  size_t _query_start; // 0 = no query in url, > 0 = query start position in url '?'

  static size_t get_query_start(std::string const& url)
  {
    auto position = url.find('?');
    return position == std::string::npos ? 0 : position;
  }

public:
  Request(http_method::HttpMethod httpMethod, std::string const& url)
  {
    this->_method = httpMethod;
    this->_url = url;
    this->_query_start = get_query_start(url);
  }

  Request(http_method::HttpMethod httpMethod, std::string const& url, std::string const& body)
      : Request(httpMethod, url)
  {
    this->_body = body;
  }

  http_method::HttpMethod getMethod();
  std::string const& getUrl();
  std::string const& getBody();
  std::map<std::string, std::string> const& getHeaders();

  void addHeader(std::string const& name, std::string const& value);
  void addQueryParameter(std::string const& name, std::string const& value);
  void addPath(std::string const& path);
};

} // namespace http
} // namespace core
} // namespace azure

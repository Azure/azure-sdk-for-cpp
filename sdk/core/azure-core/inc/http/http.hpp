// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

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

class Header
{
private:
  std::string name;
  std::string value;

public:
  Header(std::string name, std::string value)
  {
    this->name = name;
    this->value = value;
  }
  ~Header() {}
  std::string getName();
  std::string getValue();
};

class Request
{

private:
  http_method::HttpMethod method;
  std::string url;
  std::vector<Header> headers;
  std::string body;
  size_t query_start; // 0 = no query in url, > 0 = query start position in url '?'

  inline size_t get_query_start(std::string someUrl)
  {
    auto position = someUrl.find('?');
    return position == std::string::npos ? 0 : position;
  }

public:
  Request(http_method::HttpMethod httpMethod, std::string url)
  {
    this->method = httpMethod;
    this->url = url;
    this->query_start = get_query_start(url);
  }

  Request(http_method::HttpMethod httpMethod, std::string url, std::string body) : Request(httpMethod, url)
  {
    this->body = body;
  }

  ~Request() {}
  http_method::HttpMethod getMethod();
  std::string getUrl();
  std::string getBody();
  std::vector<Header> getHeaders();

  void addHeader(std::string name, std::string value);
  void addQueryParameter(std::string name, std::string value);
  void addPath(std::string path);
};

} // namespace http
} // namespace core
} // namespace azure

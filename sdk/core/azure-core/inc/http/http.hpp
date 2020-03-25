// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include <internal/contract.hpp>

using namespace std;

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
  string name;
  string value;

public:
  Header(string name, string value)
  {
    this->name = name;
    this->value = value;
  }
  ~Header() {}
  string getName();
  string getValue();
};

class Request
{

private:
  http_method::HttpMethod method;
  string url;
  std::vector<Header> headers;
  string body;
  size_t query_start; // 0 = no query in url, > 0 = query start position in url '?'

  inline size_t get_query_start(string someUrl)
  {
    auto position = someUrl.find('?');
    return position == string::npos ? 0 : position;
  }

public:
  Request(http_method::HttpMethod httpMethod, string url)
  {
    this->method = httpMethod;
    this->url = url;
    this->query_start = get_query_start(url);
  }

  Request(http_method::HttpMethod httpMethod, string url, string body) : Request(httpMethod, url)
  {
    this->body = body;
  }

  ~Request() {}
  http_method::HttpMethod getMethod();
  string getUrl();
  string getBody();
  vector<Header> getHeaders();

  void addHeader(string name, string value);
  void addQueryParameter(string name, string value);
  void addPath(string path);
};

} // namespace http
} // namespace core
} // namespace azure

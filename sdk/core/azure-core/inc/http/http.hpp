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
  string method;
  string url;
  std::vector<Header> headers;
  string body;
  int32_t query_start; // 0 = no query in url, > 0 = query start position in url '?'

  inline int32_t get_query_start(string url)
  {
    auto position = url.find('?');
    return position == string::npos ? 0 : position;
  }

public:
  Request(string httpMethod, string url)
  {
    this->method = httpMethod;
    this->url = url;
    this->query_start = get_query_start(url);
  }

  Request(string httpMethod, string url, string body) : Request(httpMethod, url)
  {
    this->body = body;
  }

  ~Request() {}
  string getMethod();
  string getUrl();
  string getBody();
  vector<Header> getHeaders();

  void addHeader(string name, string value);
  void addQueryParameter(string name, string value);
};

} // namespace http
} // namespace core
} // namespace azure

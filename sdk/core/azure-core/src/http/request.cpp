// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <map>
#include <string>

#include <http/http.hpp>

using namespace azure::core::http;

http_method::HttpMethod Request::getMethod() { return this->_method; }
std::string const& Request::getUrl() { return this->_url; }
std::string const& Request::getBody() { return this->_body; }
std::map<std::string, std::string> const& Request::getHeaders() { return this->_headers; }

void Request::addHeader(std::string const& name, std::string const& value)
{
  this->_headers.insert(std::pair<std::string, std::string>(name, value));
}

void Request::addQueryParameter(std::string const& name, std::string const& value)
{
  // Add question mark if there are not query parameters
  if (this->_query_start == 0)
  {
    this->_url = this->_url + "?";
    this->_query_start = this->_url.length();
  }
  else
  {
    this->_url = this->_url + "&";
  }

  // adding name
  this->_url = this->_url + name;
  // Add symbol
  this->_url = this->_url + "=";
  // value
  this->_url = this->_url + value;
}

void Request::addPath(std::string const& path)
{
  // save query parameters if any
  std::string queryParameters = "";
  std::string urlWithNoQuery = this->_url;
  if (this->_query_start > 0)
  {
    queryParameters = this->_url.substr(this->_query_start - 1);
    urlWithNoQuery = this->_url.substr(0, this->_query_start - 1);
  }
  this->_url = urlWithNoQuery + "/" + path;

  // update new query start
  this->_query_start = this->_query_start > 0 ? this->_url.length() + 1 : 0;

  this->_url = this->_url + queryParameters;
}

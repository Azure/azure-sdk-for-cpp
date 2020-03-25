// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <string>

#include <http/http.hpp>

using namespace azure::core::http;
using namespace std;

std::string Header::getName() { return this->name; }
std::string Header::getValue() { return this->value; }

http_method::HttpMethod Request::getMethod() { return this->method; }
string Request::getUrl() { return this->url; }
string Request::getBody() { return this->body; }
vector<Header> Request::getHeaders() { return this->headers; }

void Request::addHeader(string name, string value)
{
  try
  {
    this->headers.push_back(Header(name, value));
  }
  catch (exception e)
  {
    throw e;
  }
}

void Request::addQueryParameter(string name, string value)
{
  // Add question mark if there are not query parameters
  if (this->query_start == 0)
  {
    this->url = this->url + "?";
    this->query_start = this->url.length();
  }
  else
  {
    this->url = this->url + "&";
  }

  // adding name
  this->url = this->url + name;
  // Add symbol
  this->url = this->url + "=";
  // value
  this->url = this->url + value;
}

void Request::addPath(string path)
{
  // save query parameters if any
  std::string queryParameters = "";
  std::string urlWithNoQuery = this->url;
  if (this->query_start > 0)
  {
    queryParameters = this->url.substr(this->query_start - 1);
    urlWithNoQuery = this->url.substr(0, this->query_start - 1);
  }
  this->url = urlWithNoQuery + "/" + path;

  // update new query start
  this->query_start = this->query_start > 0 ? this->url.length() + 1 : 0;

  this->url = this->url + queryParameters;
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <string>

#include <http/http.hpp>

using namespace azure::core::http;

string Header::getName() { return this->name; }
string Header::getValue() { return this->value; }

string Request::getMethod() { return this->method; }
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

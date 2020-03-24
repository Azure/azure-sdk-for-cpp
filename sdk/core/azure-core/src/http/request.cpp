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

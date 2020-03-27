// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <map>
#include <string>
#include <vector>

#include <http/http.hpp>

using namespace azure::core::http;

void Request::addPath(std::string const& path) { this->_url += "/" + path; }

void Request::addQueryParameter(std::string const& name, std::string const& value)
{
  if (this->_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->_retryQueryParameters[name] = value;
  }
  else
  {
    this->_queryParameters.insert(std::pair<std::string, std::string>(name, value));
  }
}

void Request::addHeader(std::string const& name, std::string const& value)
{
  if (this->_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->_retryHeaders[name] = value;
  }
  else
  {
    this->_headers.insert(std::pair<std::string, std::string>(name, value));
  }
}

void Request::startRetry()
{
  this->_retryModeEnabled = true;
  this->_retryHeaders.clear();
}

HttpMethod Request::getMethod() { return this->_method; }

std::string Request::getEncodedUrl()
{
  if (this->_queryParameters.size() == 0 && this->_retryQueryParameters.size() == 0)
  {
    return _url; // no query parameters to add
  }

  // remove query duplicates
  auto queryParameters = Request::mergeMaps(this->_retryQueryParameters, this->_queryParameters);
  // build url
  auto queryString = std::string("");
  for (auto pair : queryParameters)
  {
    queryString += (queryString.empty() ? "?" : "&") + pair.first + "=" + pair.second;
  }

  return _url + queryString;
}

std::map<std::string, std::string> Request::getHeaders()
{
  // create map with retry headers witch are the most important and we don't want
  // to override them with any duplicate header
  return Request::mergeMaps(this->_retryHeaders, this->_headers);
}

BodyStream* Request::getBodyStream() { return _bodyStream; }

BodyBuffer* Request::getBodyBuffer() { return _bodyBuffer; }

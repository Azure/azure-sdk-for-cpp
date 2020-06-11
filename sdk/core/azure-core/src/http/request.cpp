// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

void Request::AddPath(std::string const& path) { this->m_url.AddPath(path); }

void Request::AddQueryParameter(std::string const& name, std::string const& value)
{
  if (this->m_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->m_retryQueryParameters[name] = value;
  }
  else
  {
    this->m_queryParameters.insert(std::pair<std::string, std::string>(name, value));
  }
}

void Request::AddHeader(std::string const& name, std::string const& value)
{
  if (this->m_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->m_retryHeaders[name] = value;
  }
  else
  {
    this->m_headers.insert(std::pair<std::string, std::string>(name, value));
  }
}

void Request::StartRetry()
{
  this->m_retryModeEnabled = true;
  this->m_retryHeaders.clear();
}

HttpMethod Request::GetMethod() const { return this->m_method; }

std::string Request::GetQueryString() const
{
  // remove query duplicates
  auto queryParameters = Request::MergeMaps(this->m_retryQueryParameters, this->m_queryParameters);
  // build url
  auto queryString = std::string("");
  for (auto pair : queryParameters)
  {
    queryString += (queryString.empty() ? "?" : "&") + pair.first + "=" + pair.second;
  }

  return queryString;
}

std::string Request::GetEncodedUrl() const
{
  if (this->m_queryParameters.size() == 0 && this->m_retryQueryParameters.size() == 0)
  {
    return m_url.ToString(); // no query parameters to add
  }

  return m_url.ToString() + GetQueryString();
}

std::map<std::string, std::string> Request::GetHeaders() const
{
  // create map with retry headers witch are the most important and we don't want
  // to override them with any duplicate header
  return Request::MergeMaps(this->m_retryHeaders, this->m_headers);
}

BodyStream* Request::GetBodyStream() { return m_bodyStream; }

std::vector<uint8_t> const& Request::GetBodyBuffer() { return m_bodyBuffer; }

// Writes an HTTP request with RFC2730 without the body (head line and headers)
// https://tools.ietf.org/html/rfc7230#section-3.1.1
std::string Request::ToString()
{
  std::string httpRequest(HttpMethodToString(this->m_method));
  // origin-form. TODO: parse URL to split host from path and use it here instead of empty
  // HTTP version harcoded to 1.0
  httpRequest += " /" + this->m_url.GetPath() + GetQueryString() + " HTTP / 1.1\r\n ";
  // headers
  for (auto header : this->GetHeaders())
  {
    httpRequest += header.first;
    httpRequest += ":";
    httpRequest += header.second;
    httpRequest += "\r\n";
  }
  // end of headers
  httpRequest += "\r\n";

  return httpRequest;
}

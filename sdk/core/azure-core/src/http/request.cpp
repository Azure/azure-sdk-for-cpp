// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/azure.hpp>
#include <azure/core/http/http.hpp>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

void Request::AppendPath(std::string const& path) { this->m_url.AppendPath(path); }

void Request::AddQueryParameter(std::string const& name, std::string const& value)
{
  if (this->m_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->m_retryQueryParameters[name] = value;
  }
  else
  {
    this->m_url.AddQueryParameter(name, value);
  }
}

void Request::AddHeader(std::string const& name, std::string const& value)
{
  auto headerNameLowerCase = Azure::Core::Details::ToLower(name);
  if (this->m_retryModeEnabled)
  {
    // When retry mode is ON, any new value must override previous
    this->m_retryHeaders[headerNameLowerCase] = value;
  }
  else
  {
    this->m_headers.insert(std::pair<std::string, std::string>(headerNameLowerCase, value));
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
  auto queryParameters
      = Request::MergeMaps(this->m_retryQueryParameters, this->m_url.GetQueryParameters());
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
  if (this->m_url.GetQueryParameters().size() == 0 && this->m_retryQueryParameters.size() == 0)
  {
    return m_url.ToString(); // no query parameters to add
  }

  return m_url.ToString() + GetQueryString();
}

std::string Request::GetHost() const { return m_url.GetHost(); }

std::map<std::string, std::string> Request::GetHeaders() const
{
  // create map with retry headers witch are the most important and we don't want
  // to override them with any duplicate header
  return Request::MergeMaps(this->m_retryHeaders, this->m_headers);
}

// Writes an HTTP request with RFC2730 without the body (head line and headers)
// https://tools.ietf.org/html/rfc7230#section-3.1.1
std::string Request::GetHTTPMessagePreBody() const
{
  std::string httpRequest(HttpMethodToString(this->m_method));
  // origin-form. TODO: parse URL to split host from path and use it here instead of empty
  // HTTP version harcoded to 1.0
  auto path = this->m_url.GetPath();
  path = path.size() > 0 ? path : "/";
  httpRequest += " " + path + GetQueryString() + " HTTP/1.1\r\n";
  // headers
  for (auto header : this->GetHeaders())
  {
    httpRequest += header.first;
    httpRequest += ": ";
    httpRequest += header.second;
    httpRequest += "\r\n";
  }
  // end of headers
  httpRequest += "\r\n";

  return httpRequest;
}

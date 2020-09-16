// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/azure.hpp>
#include <azure/core/http/http.hpp>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

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
    this->m_headers[headerNameLowerCase] = value;
  }
}

void Request::RemoveHeader(std::string const& name)
{
  this->m_headers.erase(name);
  this->m_retryHeaders.erase(name);
}

void Request::StartRetry()
{
  this->m_retryModeEnabled = true;
  this->m_retryHeaders.clear();
  this->m_url.StartRetry();
}

void Request::StopRetry()
{
  this->m_retryModeEnabled = false;
  this->m_retryHeaders.clear();
  this->m_url.StopRetry();
}

HttpMethod Request::GetMethod() const { return this->m_method; }

std::map<std::string, std::string> Request::GetHeaders() const
{
  // create map with retry headers which are the most important and we don't want
  // to override them with any duplicate header
  return Details::MergeMaps(this->m_retryHeaders, this->m_headers);
}

// Writes an HTTP request with RFC2730 without the body (head line and headers)
// https://tools.ietf.org/html/rfc7230#section-3.1.1
std::string Request::GetHTTPMessagePreBody() const
{
  std::string httpRequest(HttpMethodToString(this->m_method));
  // origin-form. TODO: parse Url to split host from path and use it here instead of empty
  // HTTP version harcoded to 1.0
  auto const url = this->m_url.GetRelativeUrl();
  httpRequest += " /" + url + " HTTP/1.1\r\n";
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

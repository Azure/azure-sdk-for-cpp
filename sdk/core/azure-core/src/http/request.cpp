// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/internal/strings.hpp"

#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

namespace {
// returns left map plus all items in right
// when duplicates, left items are preferred
static std::map<std::string, std::string> MergeMaps(
    std::map<std::string, std::string> left,
    std::map<std::string, std::string> const& right)
{
  left.insert(right.begin(), right.end());
  return left;
}
} // namespace

void Request::SetHeader(std::string const& name, std::string const& value)
{
  auto headerNameLowerCase = Azure::Core::Internal::Strings::ToLower(name);
  return this->m_retryModeEnabled
      ? Details::InsertHeaderWithValidation(this->m_retryHeaders, headerNameLowerCase, value)
      : Details::InsertHeaderWithValidation(this->m_headers, headerNameLowerCase, value);
}

void Request::RemoveHeader(std::string const& name)
{
  this->m_headers.erase(name);
  this->m_retryHeaders.erase(name);
}

void Request::StartTry()
{
  this->m_retryModeEnabled = true;
  this->m_retryHeaders.clear();

  // Make sure to rewind the body stream before each attempt, including the first.
  // It's possible the request doesn't have a body, so make sure to check if a body stream exists.
  if (auto bodyStream = this->GetBodyStream())
  {
    bodyStream->Rewind();
  }
}

HttpMethod Request::GetMethod() const { return this->m_method; }

std::map<std::string, std::string> Request::GetHeaders() const
{
  // create map with retry headers which are the most important and we don't want
  // to override them with any duplicate header
  return MergeMaps(this->m_retryHeaders, this->m_headers);
}

std::string Request::GetHeadersAsString() const
{
  std::string requestHeaderString;

  for (auto const& header : this->GetHeaders())
  {
    requestHeaderString += header.first; // string (key)
    requestHeaderString += ": ";
    requestHeaderString += header.second; // string's value
    requestHeaderString += "\r\n";
  }
  requestHeaderString += "\r\n";

  return requestHeaderString;
}

// Writes an HTTP request with RFC 7230 without the body (head line and headers)
// https://tools.ietf.org/html/rfc7230#section-3.1.1
std::string Request::GetHTTPMessagePreBody() const
{
  std::string httpRequest(HttpMethodToString(this->m_method));
  // HTTP version harcoded to 1.0
  auto const url = this->m_url.GetRelativeUrl();
  httpRequest += " /" + url + " HTTP/1.1\r\n";

  // headers
  httpRequest += GetHeadersAsString();

  return httpRequest;
}

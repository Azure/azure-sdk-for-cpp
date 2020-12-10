// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/strings.hpp"

#include <cctype>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

HttpStatusCode RawResponse::GetStatusCode() const { return m_statusCode; }

std::string const& RawResponse::GetReasonPhrase() const { return m_reasonPhrase; }

std::map<std::string, std::string> const& RawResponse::GetHeaders() const
{
  return this->m_headers;
}

void RawResponse::AddHeader(std::string::const_iterator first, std::string::const_iterator last)
{
  // get name and value from header
  auto start = first;
  auto end = std::find(start, last, ':');

  if (end == last)
  {
    throw InvalidHeaderException("Invalid header. No delimiter ':' found.");
  }

  // Always toLower() headers
  auto headerName = Azure::Core::Strings::ToLower(std::string(start, end));
  start = end + 1; // start value
  while (start < last && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  end = std::find(start, last, '\r');
  auto headerValue = std::string(start, end); // remove \r

  AddHeader(headerName, headerValue);
}

void RawResponse::AddHeader(std::string const& header)
{
  return AddHeader(header.begin(), header.end());
}

void RawResponse::AddHeader(std::string const& name, std::string const& value)
{
  return Details::InsertHeaderWithValidation(this->m_headers, name, value);
}

void RawResponse::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

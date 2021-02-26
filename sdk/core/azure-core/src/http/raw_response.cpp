// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/internal/strings.hpp"

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

void RawResponse::SetHeader(uint8_t const* const first, uint8_t const* const last)
{
  // get name and value from header
  auto start = first;
  auto end = std::find(start, last, ':');

  if (end == last)
  {
    throw InvalidHeaderException("Invalid header. No delimiter ':' found.");
  }

  // Always toLower() headers
  auto headerName = Azure::Core::Internal::Strings::ToLower(std::string(start, end));
  start = end + 1; // start value
  while (start < last && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  end = std::find(start, last, '\r');
  auto headerValue = std::string(start, end); // remove \r

  SetHeader(headerName, headerValue);
}

void RawResponse::SetHeader(std::string const& header)
{
  return SetHeader(
      reinterpret_cast<uint8_t const*>(header.data()),
      reinterpret_cast<uint8_t const*>(header.data() + header.size()));
}

void RawResponse::SetHeader(std::string const& name, std::string const& value)
{
  return Details::InsertHeaderWithValidation(this->m_headers, name, value);
}

void RawResponse::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

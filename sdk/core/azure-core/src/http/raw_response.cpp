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

void RawResponse::AddHeader(uint8_t const* const begin, uint8_t const* const last)
{
  // get name and value from header
  auto start = begin;
  auto end = std::find(start, last, ':');

  if (end == last)
  {
    throw InvalidHeaderException("invalid header. No delimiter :");
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
  return AddHeader(
      reinterpret_cast<uint8_t const*>(header.data()),
      reinterpret_cast<uint8_t const*>(header.data() + header.size()));
}

void RawResponse::AddHeader(std::string const& name, std::string const& value)
{
  return Details::InsertHeaderWithValidation(this->m_headers, name, value);
}

void RawResponse::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

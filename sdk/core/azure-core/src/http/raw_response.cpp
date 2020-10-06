// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/azure.hpp"
#include "azure/core/http/http.hpp"

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

bool RawResponse::AddHeader(
    uint8_t const* const begin,
    uint8_t const* const last,
    size_t* const invalidHeaderIndex)
{
  // get name and value from header
  auto start = begin;
  auto end = std::find(start, last, ':');

  if (end == last)
  {
    return 1; // not a valid header or end of headers symbol reached
  }

  // Always toLower() headers
  auto headerName = Azure::Core::Details::ToLower(std::string(start, end));
  start = end + 1; // start value
  while (start < last && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  end = std::find(start, last, '\r');
  auto headerValue = std::string(start, end); // remove \r

  return AddHeader(headerName, headerValue, invalidHeaderIndex);
}

bool RawResponse::AddHeader(std::string const& header, size_t* const invalidHeaderIndex)
{
  return AddHeader(
      reinterpret_cast<uint8_t const*>(header.data()),
      reinterpret_cast<uint8_t const*>(header.data() + header.size()),
      invalidHeaderIndex);
}

bool RawResponse::AddHeader(
    std::string const& name,
    std::string const& value,
    size_t* const invalidHeaderIndex)
{
  return Details::InsertHeaderWithValidation(this->m_headers, name, value, invalidHeaderIndex);
}

void RawResponse::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

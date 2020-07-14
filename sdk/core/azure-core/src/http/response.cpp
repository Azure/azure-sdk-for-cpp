// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure.hpp>
#include <cctype>
#include <http/http.hpp>

#include <cctype>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

HttpStatusCode Response::GetStatusCode() const { return m_statusCode; }

std::string const& Response::GetReasonPhrase() { return m_reasonPhrase; }

std::map<std::string, std::string> const& Response::GetHeaders() const { return this->m_headers; }

void Response::AddHeader(uint8_t const* const begin, uint8_t const* const last)
{
  // get name and value from header
  auto start = begin;
  auto end = std::find(start, last, ':');

  if (end == last)
  {
    return; // not a valid header or end of headers symbol reached
  }

  // Always toLower() headers
  // auto headerName = Azure::Core::Details::ToLower(std::string(start, end));
  auto headerName = std::string(start, end);
  start = end + 1; // start value
  while (start < last && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  end = std::find(start, last, '\r');
  auto headerValue = std::string(start, end); // remove \r

  AddHeader(headerName, headerValue);
}

void Response::AddHeader(std::string const& header)
{
  return AddHeader(
      reinterpret_cast<uint8_t const*>(header.data()),
      reinterpret_cast<uint8_t const*>(header.data() + header.size()));
}

void Response::AddHeader(std::string const& name, std::string const& value)
{

  this->m_headers.insert(std::pair<std::string, std::string>(name, value));
}

void Response::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

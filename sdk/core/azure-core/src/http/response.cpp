// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cctype>
#include <map>
#include <string>
#include <vector>

#include <http/http.hpp>

using namespace azure::core::http;

HttpStatusCode Response::GetStatusCode() { return m_statusCode; }

std::string const& Response::GetReasonPhrase() { return m_reasonPhrase; }

std::map<std::string, std::string> const& Response::GetHeaders() { return this->m_headers; }

std::vector<uint8_t>& Response::GetBodyBuffer() { return m_bodyBuffer; }

namespace
{
inline bool is_string_equals_ignore_case(std::string const& a, std::string const& b)
{
  auto alen = a.length();
  auto blen = b.length();

  if (alen != blen)
  {
    return false;
  }

  for (size_t index = 0; index < alen; index++)
  {
    // TODO: tolower is bad for some charsets, see if this can be enhanced
    if (std::tolower(a.at(index)) != std::tolower(b.at(index)))
    {
      return false;
    }
  }
  return true;
}
} // namespace

void Response::AddHeader(std::string const& name, std::string const& value)
{
  if (is_string_equals_ignore_case("Content-Length", name))
  {
    // whenever this header is found, we reserve the value of it to be pre-allocated to write
    // response
    m_bodyBuffer.reserve(std::stol(value));
  }
  this->m_headers.insert(std::pair<std::string, std::string>(name, value));
}

void Response::AppendBody(uint8_t* ptr, uint64_t size)
{
  m_bodyBuffer.insert(m_bodyBuffer.end(), ptr, ptr + size);
}

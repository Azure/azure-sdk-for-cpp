// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cctype>
#include <map>
#include <string>
#include <vector>

#include <http/http.hpp>

using namespace azure::core::http;

uint16_t Response::getStatusCode() { return m_statusCode; }

std::string const& Response::getReasonPhrase() { return m_reasonPhrase; }

std::map<std::string, std::string> const& Response::getHeaders() { return this->m_headers; }

BodyStream* Response::getBodyStream() { return m_bodyStream; }

std::vector<uint8_t>& Response::getBodyBuffer() { return m_bodyBuffer; }

std::string Response::getStringBody()
{
  return std::string(m_bodyBuffer.begin(), m_bodyBuffer.end());
}

std::string Response::getHttpVersion() { return m_httpVersion; }

namespace
{
inline bool is_string_equals_ignore_case(std::string const& a, std::string const& b)
{
  return a.length() == b.length()
      && std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char x, char y) {
           return std::tolower(x) == std::tolower(y);
         });
}
} // namespace

void Response::addHeader(std::string const& name, std::string const& value)
{
  if (is_string_equals_ignore_case("Content-Length", name))
  {
    // whenever this header is found, we reserve the value of it to be pre-allocated to write
    // response
    m_bodyBuffer.reserve(std::stol(value));
  }
  this->m_headers.insert(std::pair<std::string, std::string>(name, value));
}

void Response::appendBody(uint8_t* ptr, uint64_t size)
{
  m_bodyBuffer.insert(m_bodyBuffer.end(), ptr, ptr + size);
}

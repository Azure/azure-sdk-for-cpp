// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <map>
#include <string>
#include <vector>

#include <http/http.hpp>

using namespace Azure::Core::Http;

uint16_t Response::getStatusCode()
{
  return m_statusCode;
}

std::string const& Response::getReasonPhrase()
{
  return m_reasonPhrase;
}

std::map<std::string, std::string> const& Response::getHeaders()
{
  return this->m_headers;
}

BodyStream* Response::getBodyStream()
{
  return m_bodyStream;
}

BodyBuffer* Response::getBodyBuffer()
{
  return m_bodyBuffer;
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cctype>
#include <http/http.hpp>
#include <map>
#include <string>
#include <vector>

using namespace Azure::Core::Http;

HttpStatusCode Response::GetStatusCode() const { return m_statusCode; }

std::string const& Response::GetReasonPhrase() { return m_reasonPhrase; }

std::map<std::string, std::string> const& Response::GetHeaders() { return this->m_headers; }

void Response::AddHeader(std::string const& header)
{
  // get name and value from header
  auto start = header.begin();
  auto end = std::find(start, header.end(), ':');

  if (end == header.end())
  {
    return; // not a valid header or end of headers symbol reached
  }

  auto headerName = std::string(start, end);
  start = end + 1; // start value
  while (start < header.end() && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  end = std::find(start, header.end(), '\r');
  auto headerValue = std::string(start, end); // remove \r

  AddHeader(headerName, headerValue);
}

void Response::AddHeader(std::string const& name, std::string const& value)
{
  // TODO: make sure the Content-Length header is insterted as "Content-Length" no mather the case
  //       We currently assume we receive it like it and expected to be there from all HTTP
  //       Responses.
  this->m_headers.insert(std::pair<std::string, std::string>(name, value));
}

void Response::SetBodyStream(BodyStream* stream) { this->m_bodyStream = stream; }

std::unique_ptr<std::vector<uint8_t>> Response::ConstructBodyBufferFromStream(
    BodyStream* const stream)
{
  if (stream == nullptr)
  {
    return nullptr;
  }

  auto const bodySize = stream->Length();
  if (bodySize <= 0)
  {
    // no body to get
    return nullptr;
  }
  std::unique_ptr<std::vector<uint8_t>> unique_buffer(new std::vector<uint8_t>((size_t)bodySize));

  auto buffer = unique_buffer.get()->data();
  stream->Read(buffer, bodySize);

  return unique_buffer;
}

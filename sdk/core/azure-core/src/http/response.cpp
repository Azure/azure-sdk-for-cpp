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

void Response::SetBodyStream(std::unique_ptr<BodyStream> stream)
{
  this->m_bodyStream = std::move(stream);
}

std::unique_ptr<std::vector<uint8_t>> Response::ConstructBodyBufferFromStream(
    BodyStream* const stream)
{
  if (stream == nullptr)
  {
    return nullptr;
  }

  auto const bodySize = stream->Length();
  auto unique_buffer = std::make_unique<std::vector<uint8_t>>();

  if (bodySize >= 1)
  {
    // know the size, reserve to avoid auto-resize
    unique_buffer->reserve(bodySize);
  }

  // Loop to read
  {
    constexpr uint64_t fixedSize = 1024;
    uint8_t tempBuffer[fixedSize];
    auto readBytes = int64_t();
    do
    {
      readBytes = stream->Read(tempBuffer, fixedSize);
      unique_buffer->insert(unique_buffer->end(), tempBuffer, tempBuffer + readBytes);
    } while (readBytes != 0);
  }

  return unique_buffer;
}

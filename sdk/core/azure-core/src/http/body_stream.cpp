// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/platform.hpp"

#include "azure/core/context.hpp"
#include "azure/core/http/body_stream.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

using Azure::Core::Context;
using namespace Azure::Core::Http;

// Keep reading until buffer is all fill out of the end of stream content is reached
int64_t BodyStream::ReadToCount(
    Context const& context,
    BodyStream& body,
    uint8_t* buffer,
    int64_t count)
{
  int64_t totalRead = 0;

  for (;;)
  {
    int64_t readBytes = body.Read(context, buffer + totalRead, count - totalRead);
    totalRead += readBytes;
    // Reach all of buffer size
    if (totalRead == count || readBytes == 0)
    {
      return totalRead;
    }
  }
}

std::vector<uint8_t> BodyStream::ReadToEnd(Context const& context, BodyStream& body)
{
  constexpr int64_t chunkSize = 1024 * 8;
  auto buffer = std::vector<uint8_t>();

  for (auto chunkNumber = 0;; chunkNumber++)
  {
    buffer.resize((static_cast<decltype(buffer)::size_type>(chunkNumber) + 1) * chunkSize);
    int64_t readBytes
        = ReadToCount(context, body, buffer.data() + (chunkNumber * chunkSize), chunkSize);

    if (readBytes < chunkSize)
    {
      buffer.resize(static_cast<size_t>((chunkNumber * chunkSize) + readBytes));
      return buffer;
    }
  }
}

int64_t MemoryBodyStream::OnRead(Context const& context, uint8_t* buffer, int64_t count)
{
  (void)context;
  int64_t copy_length = std::min(count, static_cast<int64_t>(this->m_length - this->m_offset));
  // Copy what's left or just the count
  std::memcpy(buffer, this->m_data + m_offset, static_cast<size_t>(copy_length));
  // move position
  m_offset += copy_length;

  return copy_length;
}

int64_t FileBodyStream::OnRead(Azure::Core::Context const& context, uint8_t* buffer, int64_t count)
{
  (void)context;

  int64_t expectedRead = std::min(count, this->m_length - this->m_offset);
  const size_t numberOfBytesRead = fread(buffer, 1, expectedRead, this->m_hFile);

  if (numberOfBytesRead != static_cast<size_t>(expectedRead) && !feof(this->m_hFile))
  {
    throw std::runtime_error(
        "Reading error. (Code Number: " + std::to_string(ferror(this->m_hFile)) + ")");
  }

  this->m_offset += numberOfBytesRead;
  return numberOfBytesRead;
}

int64_t LimitBodyStream::OnRead(Context const& context, uint8_t* buffer, int64_t count)
{
  (void)context;
  // Read up to count or whatever length is remaining; whichever is less
  uint64_t bytesRead
      = m_inner->Read(context, buffer, std::min(count, this->m_length - this->m_bytesRead));
  this->m_bytesRead += bytesRead;
  return bytesRead;
}

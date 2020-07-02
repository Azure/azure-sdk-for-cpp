// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef POSIX
#include <unistd.h>
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // Windows

#include <algorithm>
#include <context.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <http/bodyStream.hpp>
#include <memory>
#include <vector>

using namespace Azure::Core::Http;

BodyStream::~BodyStream() {}

// Keep reading until buffer is all fill out of the end of stream content is reached
int64_t BodyStream::ReadToCount(Context& context, BodyStream& body, uint8_t* buffer, int64_t count)
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

std::unique_ptr<std::vector<uint8_t>> BodyStream::ReadToEnd(Context& context, BodyStream& body)
{
  constexpr int64_t chunkSize = 1024 * 8;
  auto unique_buffer = std::make_unique<std::vector<uint8_t>>();

  for (auto chunkNumber = 0;; chunkNumber++)
  {
    unique_buffer->resize((chunkNumber + 1) * chunkSize);
    int64_t readBytes
        = ReadToCount(context, body, unique_buffer->data() + (chunkNumber * chunkSize), chunkSize);

    if (readBytes < chunkSize)
    {
      unique_buffer->resize((chunkNumber * chunkSize) + readBytes);
      return unique_buffer;
    }
  }
}

int64_t MemoryBodyStream::Read(Context& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();

  int64_t copy_length = std::min(count, static_cast<int64_t>(this->m_length - this->m_offset));
  // Copy what's left or just the count
  std::memcpy(buffer, this->m_data + m_offset, static_cast<size_t>(copy_length));
  // move position
  m_offset += copy_length;

  return copy_length;
}

#ifdef POSIX

int64_t FileBodyStream::Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();

  auto result = pread(
      this->m_fd,
      buffer,
      std::min(count, this->m_length - this->m_offset),
      this->m_baseOffset + this->m_offset);
  this->m_offset += result;
  return result;
}
#endif

#ifdef WINDOWS

int64_t FileBodyStream::Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();

  DWORD numberOfBytesRead;
  auto o = OVERLAPPED();
  o.Offset = (DWORD)(this->m_baseOffset + this->m_offset);
  o.OffsetHigh = (DWORD)((this->m_baseOffset + this->m_offset) >> 32);

  auto result = ReadFile(
      this->m_hFile,
      buffer,
      // at most 4Gb to be read
      (DWORD)std::min(
          (uint64_t)0xFFFFFFFFUL, (uint64_t)std::min(count, (this->m_length - this->m_offset))),
      &numberOfBytesRead,
      &o);
  (void)result;

  this->m_offset += numberOfBytesRead;
  return numberOfBytesRead;
}
#endif // Windows

int64_t LimitBodyStream::Read(Context& context, uint8_t* buffer, int64_t count)
{
  (void)context;
  // Read up to count or whatever length is remaining; whichever is less
  uint64_t bytesRead
      = m_inner->Read(context, buffer, std::min(count, this->m_length - this->m_bytesRead));
  this->m_bytesRead += bytesRead;
  return bytesRead;
}

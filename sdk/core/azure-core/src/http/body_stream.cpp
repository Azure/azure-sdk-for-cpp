// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <errno.h>
#include <unistd.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#endif

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

int64_t FileBodyStream::GetFileSize(FILE* file)
{
  // Get the current file position, to reset it back, after seeking to the end.
  int64_t currentPosition = 0;
  if (fgetpos(file, &currentPosition))
  {
    throw std::runtime_error("Failed to get the file object position.");
  }

  if (fseek(file, 0, SEEK_END))
  {
    throw std::runtime_error("Failed to seek to the end of the file.");
  }

  auto fileSize = ftell(file);
  if (fileSize == -1)
  {
    throw std::runtime_error("Failed to get the size of the file.");
  }

  // Reset the file position back to what it was originally set to.
  if (fsetpos(file, &currentPosition))
  {
    throw std::runtime_error("Failed to set the file object position.");
  }

  return fileSize;
}

FileBodyStream::FileBodyStream(FILE* file, int64_t offset, int64_t length)
    : m_fileStream(file), m_baseOffset(offset), m_length(length), m_offset(0)
{
  if (file == NULL)
  {
    throw std::invalid_argument(
        "The file object cannot be null and must have been successfully opened.");
  }

  if (offset < 0 || length < 0)
  {
    throw std::invalid_argument("The file offset and size must be a non-negative number.");
  }

  int64_t fileSize = GetFileSize(m_fileStream);
  if (offset > fileSize || length > fileSize || offset + length > fileSize)
  {
    throw std::invalid_argument("The offset and length cannot be larger than the file size.");
  }

#if defined(AZ_PLATFORM_WINDOWS)
  m_fileDescriptor = _fileno(m_fileStream);
  m_filehandle = (HANDLE)_get_osfhandle(m_fileDescriptor);
#elif defined(AZ_PLATFORM_POSIX)
  m_fileDescriptor = fileno(m_fileStream);
#endif
}

FileBodyStream::FileBodyStream(FILE* file, int64_t offset)
    : m_fileStream(file), m_baseOffset(offset), m_offset(0)
{
  if (file == NULL)
  {
    throw std::invalid_argument(
        "The file object cannot be null and must have been successfully opened.");
  }

  if (offset < 0)
  {
    throw std::invalid_argument("The file offset and size must be a non-negative number.");
  }

  int64_t fileSize = GetFileSize(m_fileStream);
  if (offset > fileSize)
  {
    throw std::invalid_argument("The offset cannot be larger than the file size.");
  }

  m_length = fileSize - offset;

#if defined(AZ_PLATFORM_WINDOWS)
  m_fileDescriptor = _fileno(m_fileStream);
  m_filehandle = (HANDLE)_get_osfhandle(m_fileDescriptor);
#elif defined(AZ_PLATFORM_POSIX)
  m_fileDescriptor = fileno(m_fileStream);
#endif
}

int64_t FileBodyStream::OnRead(Azure::Core::Context const& context, uint8_t* buffer, int64_t count)
{
  (void)context;

#if defined(AZ_PLATFORM_POSIX)

  auto numberOfBytesRead = pread(
      this->m_fileDescriptor,
      buffer,
      std::min(count, this->m_length - this->m_offset),
      this->m_baseOffset + this->m_offset);

  if (result < 0)
  {

    throw std::runtime_error("Reading error. (Code Number: " + std::to_string(errno) + ")");
  }

#elif defined(AZ_PLATFORM_WINDOWS)

  DWORD numberOfBytesRead;
  auto o = OVERLAPPED();
  o.Offset = static_cast<DWORD>(this->m_baseOffset + this->m_offset);
  o.OffsetHigh = static_cast<DWORD>((this->m_baseOffset + this->m_offset) >> 32);

  auto result = ReadFile(
      this->m_filehandle,
      buffer,
      // at most 4Gb to be read
      static_cast<DWORD>(std::min(
          static_cast<uint64_t>(0xFFFFFFFFUL),
          static_cast<uint64_t>(std::min(count, (this->m_length - this->m_offset))))),
      &numberOfBytesRead,
      &o);

  if (!result)
  {
    // Check error. If EOF, return bytes read to EOF.
    auto error = GetLastError();
    if (error != ERROR_HANDLE_EOF)
    {
      throw std::runtime_error("Reading error. (Code Number: " + std::to_string(error) + ")");
    }
  }

#endif

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

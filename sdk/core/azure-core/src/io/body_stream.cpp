// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <errno.h>
#include <sys/stat.h>
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
#include "azure/core/internal/io/file_handle_holder.hpp"
#include "azure/core/internal/io/random_access_file_body_stream.hpp"
#include "azure/core/io/body_stream.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

using Azure::Core::Context;
using namespace Azure::IO;

// Keep reading until buffer is all fill out of the end of stream content is reached
int64_t BodyStream::ReadToCount(
    BodyStream& body,
    uint8_t* buffer,
    int64_t count,
    Context const& context)
{
  int64_t totalRead = 0;

  for (;;)
  {
    int64_t readBytes = body.Read(buffer + totalRead, count - totalRead, context);
    totalRead += readBytes;
    // Reach all of buffer size
    if (totalRead == count || readBytes == 0)
    {
      return totalRead;
    }
  }
}

std::vector<uint8_t> BodyStream::ReadToEnd(BodyStream& body, Context const& context)
{
  constexpr int64_t chunkSize = 1024 * 8;
  auto buffer = std::vector<uint8_t>();

  for (auto chunkNumber = 0;; chunkNumber++)
  {
    buffer.resize((static_cast<decltype(buffer)::size_type>(chunkNumber) + 1) * chunkSize);
    int64_t readBytes
        = ReadToCount(body, buffer.data() + (chunkNumber * chunkSize), chunkSize, context);

    if (readBytes < chunkSize)
    {
      buffer.resize(static_cast<size_t>((chunkNumber * chunkSize) + readBytes));
      return buffer;
    }
  }
}

int64_t MemoryBodyStream::OnRead(uint8_t* buffer, int64_t count, Context const& context)
{
  (void)context;
  int64_t copy_length = std::min(count, static_cast<int64_t>(this->m_length - this->m_offset));
  // Copy what's left or just the count
  std::memcpy(buffer, this->m_data + m_offset, static_cast<size_t>(copy_length));
  // move position
  m_offset += copy_length;

  return copy_length;
}

#if defined(AZ_PLATFORM_WINDOWS)
#pragma warning(push)
// warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

FileBodyStream::FileBodyStream(const std::string& filename) : m_offset(0)
{
  m_fileStreamHolder = fopen(filename.c_str(), "rb");
  if (m_fileStreamHolder.GetValue() == nullptr)
  {
    throw std::runtime_error("Failed to open file for reading.");
  }

#if defined(AZ_PLATFORM_WINDOWS)
  m_filehandle = (HANDLE)_get_osfhandle(_fileno(m_fileStreamHolder.GetValue()));

  LARGE_INTEGER fileSize;
  if (!GetFileSizeEx(m_filehandle, &fileSize))
  {
    throw std::runtime_error("Failed to get size of file.");
  }
  m_length = fileSize.QuadPart;

  m_parallelBodyStream = new _internal::RandomAccessFileBodyStream(m_filehandle, 0, m_length);

#elif defined(AZ_PLATFORM_POSIX)
  m_fileDescriptor = fileno(m_fileStreamHolder.GetValue());

  struct stat finfo;
  if (fstat(m_fileDescriptor, &finfo))
  {
    throw std::runtime_error("Failed to get size of file.");
  }
  m_length = finfo.st_size;

  m_parallelBodyStream = new _internal::RandomAccessFileBodyStream(m_fileDescriptor, 0, m_length);

#endif
}

#if defined(AZ_PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

FileBodyStream::~FileBodyStream()
{
  delete m_parallelBodyStream;
  // The file handle is closed by the FileHandleHolder so it doesn't need to be closed here.
}

int64_t FileBodyStream::OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context)
{
  return m_parallelBodyStream->Read(buffer, count, context);
}

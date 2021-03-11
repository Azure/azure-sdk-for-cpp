// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "azure/core/context.hpp"
#include "azure/core/io/body_stream.hpp"

#include <algorithm>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

using Azure::Core::Context;
using namespace Azure::Core::IO;

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

FileBodyStream::FileBodyStream(const std::string& filename)
    : m_randomAccessFileBodyStream(
        NULL,
        0,
        0) // Required as a placeholder because no default ctor exists.
{
#if defined(AZ_PLATFORM_WINDOWS)

  try
  {
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
    m_filehandle = CreateFile(
        filename.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, // Using this as an optimization since we know file access is
                                   // intended to be sequential from beginning to end.
        NULL);
#else
    m_filehandle = CreateFile2(
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(filename).c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        OPEN_EXISTING,
        NULL);
#endif

    if (m_filehandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open file for reading. File name: '" + filename + "'");
    }
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(m_filehandle, &fileSize))
    {
      throw std::runtime_error("Failed to get size of file. File name: '" + filename + "'");
    }
    m_randomAccessFileBodyStream
        = _internal::RandomAccessFileBodyStream(m_filehandle, 0, fileSize.QuadPart);
  }
  catch (std::exception&)
  {
    CloseHandle(m_filehandle);
    throw;
  }

#elif defined(AZ_PLATFORM_POSIX)

  try
  {
    m_fileDescriptor = open(filename.data(), O_RDONLY);
    if (m_fileDescriptor == -1)
    {
      throw std::runtime_error("Failed to open file for reading. File name: '" + filename + "'");
    }
    int64_t fileSize = lseek(m_fileDescriptor, 0, SEEK_END);
    if (fileSize == -1)
    {
      throw std::runtime_error("Failed to get size of file. File name: '" + filename + "'");
    }

    m_randomAccessFileBodyStream
        = _internal::RandomAccessFileBodyStream(m_fileDescriptor, 0, finfo.st_size);
  }
  catch (std::exception&)
  {
    close(m_fileDescriptor);
    throw;
  }

#endif
}

FileBodyStream::~FileBodyStream()
{
#if defined(AZ_PLATFORM_WINDOWS)
  CloseHandle(m_filehandle);
#elif defined(AZ_PLATFORM_POSIX)
  close(m_fileDescriptor);
#endif
}

int64_t FileBodyStream::OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context)
{
  return m_randomAccessFileBodyStream.Read(buffer, count, context);
}

void FileBodyStream::Rewind() { m_randomAccessFileBodyStream.Rewind(); }

int64_t FileBodyStream::Length() const { return m_randomAccessFileBodyStream.Length(); };

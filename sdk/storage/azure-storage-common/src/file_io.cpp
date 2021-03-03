// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/file_io.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_POSIX)
#include <unistd.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#include <io.h>
#pragma warning(push)
// warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s
// instead.
#pragma warning(disable : 4996)
#endif

#include <stdexcept>
#include <stdio.h>

namespace Azure { namespace Storage { namespace Details {

  FileReader::FileReader(const std::string& filename)
  {
    FILE* handle = fopen(filename.c_str(), "rb");
    if (handle == nullptr)
    {
      throw std::runtime_error("Failed to open file for reading.");
    }

    m_handle = handle;
  }

  FileReader::~FileReader() { fclose(m_handle); }

  FileWriter::FileWriter(const std::string& filename)
  {
    FILE* handle = fopen(filename.c_str(), "wb");
    if (handle == nullptr)
    {
      throw std::runtime_error("Failed to open file for writing.");
    }

    m_handle = handle;
  }

#if defined(AZ_PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

  FileWriter::~FileWriter() { fclose(m_handle); }

  void FileWriter::Write(const uint8_t* buffer, int64_t length, int64_t offset)
  {
#if defined(AZ_PLATFORM_WINDOWS)

    if (length > std::numeric_limits<DWORD>::max())
    {
      throw std::runtime_error("failed to write file");
    }

    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = static_cast<DWORD>(static_cast<uint64_t>(offset));
    overlapped.OffsetHigh = static_cast<DWORD>(static_cast<uint64_t>(offset) >> 32);

    DWORD bytesWritten;
    BOOL ret = WriteFile(
        (HANDLE)_get_osfhandle(_fileno(m_handle)),
        buffer,
        static_cast<DWORD>(length),
        &bytesWritten,
        &overlapped);
    if (!ret)
    {
      throw std::runtime_error("failed to write file");
    }

#elif defined(AZ_PLATFORM_POSIX)

    if (static_cast<uint64_t>(length) > std::numeric_limits<size_t>::max()
        || offset > static_cast<int64_t>(std::numeric_limits<off_t>::max()))
    {
      throw std::runtime_error("failed to write file");
    }
    ssize_t bytesWritten
        = pwrite(fileno(m_handle), buffer, static_cast<size_t>(length), static_cast<off_t>(offset));
    if (bytesWritten != length)
    {
      throw std::runtime_error("failed to write file");
    }

#endif
  }
}}} // namespace Azure::Storage::Details

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/file_io.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_POSIX)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <limits>
#include <stdexcept>

namespace Azure { namespace Storage { namespace Details {

#if defined(AZ_PLATFORM_WINDOWS)
  FileReader::FileReader(const std::string& filename)
  {
#if !defined(AZ_PLATFORM_WINDOWS_UWP)
    m_handle = CreateFile(
        filename.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
#else
    m_handle = CreateFile2(filename.data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, NULL);
#endif
    if (m_handle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("failed to open file");
    }

    LARGE_INTEGER fileSize;
    BOOL ret = GetFileSizeEx(m_handle, &fileSize);
    if (!ret)
    {
      CloseHandle(m_handle);
      throw std::runtime_error("failed to get size of file");
    }
    m_fileSize = fileSize.QuadPart;
  }

  FileReader::~FileReader() { CloseHandle(m_handle); }

  FileWriter::FileWriter(const std::string& filename)
  {
#if !defined(AZ_PLATFORM_WINDOWS_UWP)
    m_handle = CreateFile(
        filename.data(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
#else
    m_handle = CreateFile2(
        filename.data(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, CREATE_ALWAYS, NULL);
#endif
    if (m_handle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("failed to open file");
    }
  }

  FileWriter::~FileWriter() { CloseHandle(m_handle); }

  void FileWriter::Write(const uint8_t* buffer, int64_t length, int64_t offset)
  {
    if (length > std::numeric_limits<DWORD>::max())
    {
      throw std::runtime_error("failed to write file");
    }

    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = static_cast<DWORD>(static_cast<uint64_t>(offset));
    overlapped.OffsetHigh = static_cast<DWORD>(static_cast<uint64_t>(offset) >> 32);

    DWORD bytesWritten;
    BOOL ret = WriteFile(m_handle, buffer, static_cast<DWORD>(length), &bytesWritten, &overlapped);
    if (!ret)
    {
      throw std::runtime_error("failed to write file");
    }
  }
#elif defined(AZ_PLATFORM_POSIX)
  FileReader::FileReader(const std::string& filename)
  {
    m_handle = open(filename.data(), O_RDONLY);
    if (m_handle == -1)
    {
      throw std::runtime_error("failed to open file");
    }
    m_fileSize = lseek(m_handle, 0, SEEK_END);
    if (m_fileSize == -1)
    {
      close(m_handle);
      throw std::runtime_error("failed to get size of file");
    }
  }

  FileReader::~FileReader() { close(m_handle); }

  FileWriter::FileWriter(const std::string& filename)
  {
    m_handle = open(
        filename.data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (m_handle == -1)
    {
      throw std::runtime_error("failed to open file");
    }
  }

  FileWriter::~FileWriter() { close(m_handle); }

  void FileWriter::Write(const uint8_t* buffer, int64_t length, int64_t offset)
  {
    if (static_cast<uint64_t>(length) > std::numeric_limits<size_t>::max()
        || offset > static_cast<int64_t>(std::numeric_limits<off_t>::max()))
    {
      throw std::runtime_error("failed to write file");
    }
    ssize_t bytesWritten
        = pwrite(m_handle, buffer, static_cast<size_t>(length), static_cast<off_t>(offset));
    if (bytesWritten != length)
    {
      throw std::runtime_error("failed to write file");
    }
  }
#endif

}}} // namespace Azure::Storage::Details

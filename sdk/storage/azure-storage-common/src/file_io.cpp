// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/internal/file_io.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_POSIX)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace Azure { namespace Storage { namespace _internal {

#if defined(AZ_PLATFORM_WINDOWS)
  std::wstring Utf8ToWide(const std::string& narrow)
  {
    int sizeNeeded = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        narrow.data(),
        static_cast<int>(narrow.length()),
        nullptr,
        0);
    if (sizeNeeded == 0)
    {
      throw std::runtime_error("Failed to convert utf8 to wide chars.");
    }
    std::wstring wide(sizeNeeded, L'\0');
    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            narrow.data(),
            static_cast<int>(narrow.length()),
            &wide[0],
            sizeNeeded)
        == 0)
    {
      throw std::runtime_error("Failed to convert utf8 to wide chars.");
    }
    return wide;
  }

  std::string Utf8ToNarrow(const std::wstring& wide)
  {
    int sizeNeeded = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        &wide[0],
        static_cast<int>(wide.length()),
        NULL,
        0,
        NULL,
        NULL);
    if (sizeNeeded == 0)
    {
      throw std::runtime_error("Failed to convert utf8 to multi-bytes.");
    }
    std::string narrow(sizeNeeded, '\0');
    if (WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            &wide[0],
            static_cast<int>(wide.length()),
            &narrow[0],
            sizeNeeded,
            NULL,
            NULL)
        == 0)
    {
      throw std::runtime_error("Failed to convert utf8 to multi-bytes.");
    }
    return narrow;
  }

  FileReader::FileReader(const std::string& filename)
  {
    const std::wstring filenameW = Utf8ToWide(filename);

    HANDLE fileHandle;

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
    fileHandle = CreateFileW(
        filenameW.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
#else
    fileHandle = CreateFile2(filenameW.data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, NULL);
#endif
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open file.");
    }

    LARGE_INTEGER fileSize;
    BOOL ret = GetFileSizeEx(fileHandle, &fileSize);
    if (!ret)
    {
      CloseHandle(fileHandle);
      throw std::runtime_error("Failed to get size of file.");
    }
    m_handle = static_cast<void*>(fileHandle);
    m_fileSize = fileSize.QuadPart;
  }

  FileReader::~FileReader() { CloseHandle(static_cast<HANDLE>(m_handle)); }

  size_t FileReader::Read(uint8_t* buffer, size_t length, int64_t offset) const
  {
    length = std::min(length, static_cast<size_t>(std::max(0LL, m_fileSize - offset)));
    if (length > std::numeric_limits<DWORD>::max())
    {
      throw std::runtime_error("Failed to read file.");
    }

    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = static_cast<DWORD>(static_cast<uint64_t>(offset));
    overlapped.OffsetHigh = static_cast<DWORD>(static_cast<uint64_t>(offset) >> 32);

    DWORD bytesRead;
    BOOL ret = ReadFile(
        static_cast<HANDLE>(m_handle), buffer, static_cast<DWORD>(length), &bytesRead, &overlapped);
    if (!ret)
    {
      throw std::runtime_error("Failed to read file.");
    }
    return bytesRead;
  }

  FileWriter::FileWriter(const std::string& filename, bool truncate)
  {
    DWORD creationDisposition = truncate ? CREATE_ALWAYS : OPEN_ALWAYS;
    const std::wstring filenameW = Utf8ToWide(filename);

    HANDLE fileHandle;

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
    fileHandle = CreateFileW(
        filenameW.data(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        creationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
#else
    fileHandle = CreateFile2(
        filenameW.data(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        creationDisposition,
        NULL);
#endif
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open file.");
    }
    m_handle = static_cast<void*>(fileHandle);
  }

  FileWriter::~FileWriter() { CloseHandle(static_cast<HANDLE>(m_handle)); }

  void FileWriter::Write(const uint8_t* buffer, size_t length, int64_t offset) const
  {
    if (length > std::numeric_limits<DWORD>::max())
    {
      throw std::runtime_error("Failed to write file.");
    }

    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = static_cast<DWORD>(static_cast<uint64_t>(offset));
    overlapped.OffsetHigh = static_cast<DWORD>(static_cast<uint64_t>(offset) >> 32);

    DWORD bytesWritten;
    BOOL ret = WriteFile(
        static_cast<HANDLE>(m_handle),
        buffer,
        static_cast<DWORD>(length),
        &bytesWritten,
        &overlapped);
    if (!ret)
    {
      throw std::runtime_error("Failed to write file.");
    }
  }
#elif defined(AZ_PLATFORM_POSIX)
  FileReader::FileReader(const std::string& filename)
  {
    m_handle = open(filename.data(), O_RDONLY);
    if (m_handle == -1)
    {
      throw std::runtime_error("Failed to open file.");
    }
    m_fileSize = lseek(m_handle, 0, SEEK_END);
    if (m_fileSize == -1)
    {
      close(m_handle);
      throw std::runtime_error("Failed to get size of file.");
    }
  }

  FileReader::~FileReader() { close(m_handle); }

  size_t FileReader::Read(uint8_t* buffer, size_t length, int64_t offset) const
  {
    if (offset > static_cast<int64_t>(std::numeric_limits<off_t>::max()))
    {
      throw std::runtime_error("Failed to read file.");
    }
    length = std::min<size_t>(length, m_fileSize - offset);
    ssize_t bytesRead = pread(m_handle, buffer, length, static_cast<off_t>(offset));
    if (bytesRead < 0)
    {
      throw std::runtime_error("Failed to read file.");
    }
    return bytesRead;
  }

  FileWriter::FileWriter(const std::string& filename, bool truncate)
  {
    int flags = O_WRONLY | O_CREAT;
    if (truncatge)
    {
      flags |= O_TRUNC;
    }
    m_handle = open(filename.data(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (m_handle == -1)
    {
      throw std::runtime_error("Failed to open file.");
    }
  }

  FileWriter::~FileWriter() { close(m_handle); }

  void FileWriter::Write(const uint8_t* buffer, size_t length, int64_t offset) const
  {
    if (offset > static_cast<int64_t>(std::numeric_limits<off_t>::max()))
    {
      throw std::runtime_error("Failed to write file.");
    }
    ssize_t bytesWritten = pwrite(m_handle, buffer, length, static_cast<off_t>(offset));
    if (bytesWritten < 0 || static_cast<size_t>(bytesWritten) != length)
    {
      throw std::runtime_error("Failed to write file.");
    }
  }
#endif

}}} // namespace Azure::Storage::_internal

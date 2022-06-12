// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/filesystem.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <climits>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstring>
#include <cwchar>
#include <memory>
#include <stdexcept>
#include <string>

#if defined(AZ_PLATFORM_WINDOWS)
namespace Azure { namespace Storage { namespace _internal {
  // cSpell:ignore DATAW
#undef CreateDirectory
  std::wstring Utf8ToWide(const std::string& narrow);
  std::string Utf8ToNarrow(const std::wstring& wide);
}}} // namespace Azure::Storage::_internal
#endif

namespace Azure { namespace Storage { namespace _internal {
#if defined(AZ_PLATFORM_WINDOWS)

  bool IsDirectory(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    DWORD ret = GetFileAttributesW(pathW.data());
    return ret != INVALID_FILE_ATTRIBUTES && (ret & FILE_ATTRIBUTE_DIRECTORY);
  }

  bool IsRegularFile(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    DWORD ret = GetFileAttributesW(pathW.data());
    return ret != INVALID_FILE_ATTRIBUTES && !(ret & FILE_ATTRIBUTE_DIRECTORY);
  }

  bool PathExists(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    DWORD ret = GetFileAttributesW(pathW.data());
    return ret != INVALID_FILE_ATTRIBUTES;
  }

  void CreateDirectory(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    BOOL ret = CreateDirectoryW(pathW.data(), NULL);
    if (ret == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
      throw std::runtime_error("Failed to create directory " + path + ".");
    }
  }

  void Rename(const std::string& oldPath, const std::string& newPath)
  {
    const std::wstring oldPathW = Utf8ToWide(oldPath);
    const std::wstring newPathW = Utf8ToWide(newPath);
    BOOL ret = MoveFileW(oldPathW.data(), newPathW.data());
    if (ret == 0)
    {
      throw std::runtime_error("Failed to move " + oldPath + " to " + newPath);
    }
  }

  void Remove(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    DWORD ret = GetFileAttributesW(pathW.data());
    if (ret == INVALID_FILE_ATTRIBUTES)
    {
      return;
    }
    if (ret & FILE_ATTRIBUTE_DIRECTORY)
    {
      RemoveDirectoryW(pathW.data());
    }
    else
    {
      DeleteFileW(pathW.data());
    }
  }

  int64_t GetFileSize(const std::string& path)
  {
    const std::wstring pathW = Utf8ToWide(path);
    WIN32_FILE_ATTRIBUTE_DATA info;
    BOOL ret = GetFileAttributesExW(pathW.data(), GetFileExInfoStandard, &info);
    if (ret == 0)
    {
      throw std::runtime_error("Failed to get file size.");
    }
    return (static_cast<uint64_t>(info.nFileSizeHigh) << 32) | info.nFileSizeLow;
  }

  struct ListDirectoryContext
  {
    HANDLE DirectoryHandle = INVALID_HANDLE_VALUE;
    DirectoryIterator::DirectoryEntry Buffer;

    ~ListDirectoryContext()
    {
      if (DirectoryHandle != INVALID_HANDLE_VALUE)
      {
        FindClose(DirectoryHandle);
      }
    }
  };

  DirectoryIterator::DirectoryIterator(const std::string& rootDirectory)
  {
    const std::wstring rootDirectoryW = Utf8ToWide(rootDirectory + "/*");

    auto context = std::make_unique<ListDirectoryContext>();

    WIN32_FIND_DATAW entry;
    context->DirectoryHandle = FindFirstFileW(rootDirectoryW.data(), &entry);
    if (context->DirectoryHandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open directory.");
    }
    if (std::wcscmp(entry.cFileName, L".") != 0 && std::wcscmp(entry.cFileName, L"..") != 0)
    {
      context->Buffer.Name = Utf8ToNarrow(entry.cFileName);
      context->Buffer.IsDirectory = entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    }
    m_directroyObject = context.release();
  }

  DirectoryIterator::~DirectoryIterator()
  {
    if (m_directroyObject)
    {
      delete static_cast<ListDirectoryContext*>(m_directroyObject);
    }
  }

  DirectoryIterator::DirectoryEntry DirectoryIterator::Next()
  {
    auto context = static_cast<ListDirectoryContext*>(m_directroyObject);
    if (!context->Buffer.Name.empty())
    {
      auto e = std::move(context->Buffer);
      context->Buffer.Name.clear();
      return e;
    }

    WIN32_FIND_DATAW entry;
    BOOL ret = FindNextFileW(context->DirectoryHandle, &entry);
    if (!ret && GetLastError() == ERROR_NO_MORE_FILES)
    {
      return DirectoryEntry();
    }
    else if (!ret)
    {
      throw std::runtime_error("Failed to list directory.");
    }

    if (std::wcscmp(entry.cFileName, L".") == 0 || std::wcscmp(entry.cFileName, L"..") == 0)
    {
      return Next();
    }

    DirectoryEntry e;
    e.Name = Utf8ToNarrow(entry.cFileName);
    e.IsDirectory = entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    e.Size = (static_cast<int64_t>(entry.nFileSizeHigh) << 32) | entry.nFileSizeLow;
    return e;
  }

  MemoryMap::MemoryMap(const std::string& filename)
  {
    const std::wstring filenameW = Utf8ToWide(filename);

    HANDLE fileHandle = CreateFileW(
        filenameW.data(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open file.");
    }
    m_fileHandle = fileHandle;
  }

  MemoryMap& MemoryMap::operator=(MemoryMap&& other) noexcept
  {
    m_fileHandle = other.m_fileHandle;
    other.m_fileHandle = nullptr;
    m_mapped = std::move(other.m_mapped);
    other.m_mapped.clear();
    return *this;
  }

  MemoryMap::~MemoryMap()
  {
    for (const auto& pair : m_mapped)
    {
      UnmapViewOfFile(pair.first);
    }
    if (m_fileHandle)
    {
      HANDLE fileHandle = m_fileHandle;
      CloseHandle(fileHandle);
    }
  }

  void* MemoryMap::Map(size_t offset, size_t size)
  {
    static const size_t Granularity = []() -> size_t {
      SYSTEM_INFO info;
      GetSystemInfo(&info);
      return static_cast<size_t>(info.dwAllocationGranularity);
    }();

    size_t alignedOffset = offset / Granularity * Granularity;
    size += offset - alignedOffset;

    HANDLE handle = CreateFileMappingW(m_fileHandle, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (handle == NULL)
    {
      throw std::runtime_error("Failed to create file mapping.");
    }

    void* ptr = MapViewOfFileEx(
        handle, FILE_MAP_ALL_ACCESS, alignedOffset >> 32, alignedOffset & 0xffffffff, size, NULL);
    CloseHandle(handle);
    if (!ptr)
    {
      throw std::runtime_error("Failed to map view of file.");
    }
    m_mapped.push_back(std::make_pair(ptr, size));
    return static_cast<char*>(ptr) + (offset - alignedOffset);
  }

#else

  void CreateDirectory(const std::string& directoryPath)
  {
    int ret = mkdir(directoryPath.data(), 0755);
    if (ret != 0 && errno != EEXIST)
    {
      throw std::runtime_error("Failed to create directory " + directoryPath + ".");
    }
  }

  struct ListDirectoryContext
  {
    DIR* DirectoryPointer = nullptr;

    ~ListDirectoryContext()
    {
      if (DirectoryPointer != nullptr)
      {
        closedir(DirectoryPointer);
      }
    }
  };

  DirectoryIterator::DirectoryIterator(const std::string& rootDirectory)
  {
    auto context = std::make_unique<ListDirectoryContext>();
    context->DirectoryPointer = opendir(rootDirectory.data());
    if (context->DirectoryPointer == nullptr)
    {
      throw std::runtime_error("Failed to open directory.");
    }
    m_directroyObject = context.release();
  }
  DirectoryIterator::~DirectoryIterator()
  {
    if (m_directroyObject)
    {
      delete static_cast<ListDirectoryContext*>(m_directroyObject);
    }
  }

  DirectoryIterator::DirectoryEntry DirectoryIterator::Next()
  {
    auto context = static_cast<ListDirectoryContext*>(m_directroyObject);
    errno = 0;
    struct dirent* entry = readdir(context->DirectoryPointer);
    if (entry == nullptr && errno == 0)
    {
      return DirectoryEntry();
    }
    else if (entry == nullptr)
    {
      throw std::runtime_error("Failed to list directory.");
    }
    if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
    {
      return Next();
    }

    DirectoryEntry e;
    e.Name = entry->d_name;
    e.IsDirectory = entry->d_type & DT_DIR;
    // TODO: size
    return e;
  }
#endif
}}} // namespace Azure::Storage::_internal

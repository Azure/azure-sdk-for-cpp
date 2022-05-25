// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/directory_iterator.hpp"

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
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#endif

#include <cstring>
#include <cwchar>
#include <memory>
#include <stdexcept>

namespace Azure { namespace Storage { namespace _internal {
#if defined(AZ_PLATFORM_WINDOWS)

  // cSpell:ignore DATAW

  std::wstring Utf8ToWide(const std::string& narrow);
  std::string Utf8ToNarrow(const std::wstring& wide);

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
    return e;
  }

#else
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
    return e;
  }
#endif
}}} // namespace Azure::Storage::_internal

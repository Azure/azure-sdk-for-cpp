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
#include <sys/types.h>
#endif

#include <cwchar>
#include <memory>
#include <stdexcept>

#if defined(AZ_PLATFORM_WINDOWS)
namespace Azure { namespace Storage { namespace _internal {
  std::wstring Utf8ToWide(const std::string& narrow);
  std::string Utf8ToNarrow(const std::wstring& wide);
}}} // namespace Azure::Storage::_internal
#endif

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {
#if defined(AZ_PLATFORM_WINDOWS)

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
    const std::wstring rootDirectoryW = Storage::_internal::Utf8ToWide(rootDirectory + "/*");

    auto context = std::make_unique<ListDirectoryContext>();

    WIN32_FIND_DATAW entry;
    context->DirectoryHandle = FindFirstFileW(rootDirectoryW.data(), &entry);
    if (context->DirectoryHandle == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("Failed to open directory.");
    }
    if (std::wcscmp(entry.cFileName, L".") != 0 && std::wcscmp(entry.cFileName, L"..") != 0)
    {
      context->Buffer.Name = Storage::_internal::Utf8ToNarrow(entry.cFileName);
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
    e.Name = Storage::_internal::Utf8ToNarrow(entry.cFileName);
    e.IsDirectory = entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    return e;
  }

#else
  DirectoryIterator::DirectoryIterator(const std::string& rootDirectory) {}
  DirectoryIterator::~DirectoryIterator() {}

  std::string DirectoryIterator::Next() {}
#endif
}}}} // namespace Azure::Storage::DataMovement::_internal

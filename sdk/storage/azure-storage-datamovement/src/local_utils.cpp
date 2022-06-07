// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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
#include <sys/stat.h>
#endif

#include <stdexcept>
#include <azure/storage/datamovement/local_utils.hpp>

namespace Azure { namespace Storage { namespace _internal {

#if defined(AZ_PLATFORM_WINDOWS)
  extern const char FolderDelimiter = '\\';
  std::wstring Utf8ToWide(const std::string& narrow);
  std::string Utf8ToNarrow(const std::wstring& wide);

  void Local_Utils::create_directory(std::string& path)
  {
    const std::wstring pathw = Utf8ToWide(path);

    DWORD fileAttributes = GetFileAttributesW(pathw.data());

    if ((fileAttributes != INVALID_FILE_ATTRIBUTES) && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      return; // diectory already exists.

    if (!CreateDirectoryW(pathw.data(), NULL))
    {
      DWORD errorNo = GetLastError();
      if (183 == errorNo)
      {
        throw std::runtime_error("A file with the same name exists.");
      }
      else
      {
        throw std::runtime_error("Failed to create directory.");
      }
    }
  }
#elif defined(AZ_PLATFORM_POSIX)
  extern const char FolderDelimiter = '/';

  void Local_Utils::create_directory(std::string& path)
  {
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
    if (0!= ret)
    {
      throw std::runtime_error("Failed to create directory.");
    }
  }
#endif
}}} // namespace Azure::Storage::_internal

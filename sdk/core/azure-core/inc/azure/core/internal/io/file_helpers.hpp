// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Internal utility functions for files and directories.
 *
 */
#pragma once

#include <azure/core/platform.hpp>

#include <stdexcept>
#include <string>

#include <sys/stat.h> // for stat() used to check file size and mkdir() used to create directories

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(AZ_PLATFORM_LINUX)
#include <unistd.h>

#include <sys/types.h>
#endif

namespace Azure { namespace Core { namespace IO { namespace _internal {

  /**
   * @brief Add basic filesystem functionality.
   */
  class FileHelpers final {
  public:
    /**
     * @brief Get the size of the file.
     *
     * @param filePath The full path to the file to examine.
     *
     * @remark This function is analogous to https://en.cppreference.com/w/cpp/filesystem/file_size,
     * which is only available in C++17.
     *
     * @return The size of the file, in bytes.
     */
    static uintmax_t GetFileSize(std::string filePath)
    {
      struct stat s;
      if (!stat(filePath.c_str(), &s))
      {
        return s.st_size;
      }
      else
      {
        throw std::runtime_error(
            "Failed to get size of file. File name: '" + filePath + "'. " + std::strerror(errno));
      }
    }

    /**
     * @brief Create the directory on disk.
     *
     * @param directoryPath The full path to the directory to create.
     *
     * @remark The parent directory must exist.
     *
     * @remark This function is analogous to
     * https://en.cppreference.com/w/cpp/filesystem/create_directory, which is only available in
     * C++17.
     *
     * @return `true` if the directory was created successfully, `false` otherwise.
     */
    static bool CreateFileDirectory(std::string directoryPath)
    {
#if defined(AZ_PLATFORM_WINDOWS)
      // Unlike linux, we can't use mkdir on Windows, since it is deprecated. We will use
      // CreateDirectory instead.
      // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/mkdir?view=msvc-170
      int result = CreateDirectory(directoryPath.c_str(), NULL);
      if (!result)
      {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
          return false;
        }
        throw std::runtime_error(
            "Failed to create directory. Directory path: '" + directoryPath + "'. "
            + std::to_string(GetLastError()));
      }
      return true;
#else
      int result = mkdir(directoryPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR);
      if (!result)
      {
        if (errno == EEXIST)
        {
          return false;
        }
        throw std::runtime_error(
            "Failed to create directory. Directory path: '" + directoryPath + "'. "
            + std::strerror(errno));
      }
      return true;
#endif
    }
  };
}}}} // namespace Azure::Core::IO::_internal

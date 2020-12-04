// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/platform.hpp>

#ifdef AZ_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <cstdint>
#include <string>

namespace Azure { namespace Storage { namespace Details {

#ifdef AZ_PLATFORM_WINDOWS
  using FileHandle = HANDLE;
#elif defined(AZ_PLATFORM_POSIX)
  using FileHandle = int;
#endif

  class FileReader {
  public:
    FileReader(const std::string& filename);

    ~FileReader();

    FileHandle GetHandle() const { return m_handle; }

    int64_t GetFileSize() const { return m_fileSize; }

  private:
    FileHandle m_handle;
    int64_t m_fileSize;
  };

  class FileWriter {
  public:
    FileWriter(const std::string& filename);

    ~FileWriter();

    FileHandle GetHandle() const { return m_handle; }

    void Write(const uint8_t* buffer, int64_t length, int64_t offset);

  private:
    FileHandle m_handle;
  };

}}} // namespace Azure::Storage::Details

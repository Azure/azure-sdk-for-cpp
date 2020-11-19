// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

#include <cstdint>
#include <string>

namespace Azure { namespace Storage { namespace Details {

#ifdef _WIN32
  using FileHandle = HANDLE;
#else
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

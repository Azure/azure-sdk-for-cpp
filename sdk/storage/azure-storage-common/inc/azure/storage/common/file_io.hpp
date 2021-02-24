// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <cstdint>
#include <string>

namespace Azure { namespace Storage { namespace Details {

  using FileHandle = FILE*;

  class FileReader {
  public:
    FileReader(const std::string& filename);

    ~FileReader();

    FileHandle GetHandle() const { return m_handle; }

  private:
    FileHandle m_handle;
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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/file_io.hpp"

#include <stdexcept>
#include <stdio.h>

namespace Azure { namespace Storage { namespace Details {

  FileReader::FileReader(const std::string& filename)
  {
    FILE* handle{};
    if (fopen_s(&handle, filename.c_str(), "rb"))
    {
      throw std::runtime_error("Failed to open file for reading.");
    }

    m_handle = handle;
  }

  FileReader::~FileReader() { fclose(m_handle); }

  FileWriter::FileWriter(const std::string& filename)
  {
    FILE* handle{};
    if (fopen_s(&handle, filename.c_str(), "wb"))
    {
      throw std::runtime_error("Failed to open file for writing.");
    }

    m_handle = handle;
  }

  FileWriter::~FileWriter() { fclose(m_handle); }

  void FileWriter::Write(const uint8_t* buffer, int64_t length, int64_t offset)
  {
    (void)offset;
    const size_t numberOfBytesWritten = fwrite(buffer, sizeof(uint8_t), length, m_handle);

    if (numberOfBytesWritten != static_cast<size_t>(length))
    {
      throw std::runtime_error(
          "Writing error. (Code Number: " + std::to_string(ferror(m_handle)) + ")");
    }
  }
}}} // namespace Azure::Storage::Details

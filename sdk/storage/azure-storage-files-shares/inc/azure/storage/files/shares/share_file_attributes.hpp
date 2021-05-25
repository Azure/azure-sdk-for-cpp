// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "azure/storage/files/shares/dll_import_export.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  class FileAttributes final {
  public:
    FileAttributes() = default;
    explicit FileAttributes(const std::string& value);

    std::string ToString() const;
    const std::vector<std::string> GetValues() const { return m_value; }

    bool operator==(const FileAttributes& other) const { return m_value == other.m_value; }
    bool operator!=(const FileAttributes& other) const { return !(*this == other); }

    FileAttributes operator|(const FileAttributes& other) const;
    FileAttributes operator&(const FileAttributes& other) const;
    FileAttributes operator^(const FileAttributes& other) const;

    FileAttributes& operator|=(const FileAttributes& other)
    {
      *this = *this | other;
      return *this;
    }

    FileAttributes& operator&=(const FileAttributes& other)
    {
      *this = *this & other;
      return *this;
    }

    FileAttributes& operator^=(const FileAttributes& other)
    {
      *this = *this ^ other;
      return *this;
    }

    /**
     * @brief The File or Directory is read-only.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes ReadOnly;

    /**
     * @brief The File or Directory is hidden, and thus is not included in an ordinary directory
     * listing.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes Hidden;

    /**
     * @brief The File or Directory is a systemfile.  That is, the file is part of the operating
     * system or is used exclusively by the operating system.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes System;

    /**
     * @brief The file  or directory is a standard file that has no special attributes. This
     * attribute is valid only if it is used alone.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes None;

    /**
     * @brief The file is a directory.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes Directory;

    /**
     * @brief The file is a candidate for backup or removal.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes Archive;

    /**
     * @brief The file or directory is temporary. A temporary file contains data that is needed
     * while an application is executing but is not needed after the application is finished. File
     * systems try to keep all the data in memory for quicker access rather than flushing the data
     * back to mass storage. A temporary file should be deleted by the application as soon as it
     * is no longer needed.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes Temporary;

    /**
     * @brief The file or directory is offline. The data of the file is not immediately available.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes Offline;

    /**
     * @brief The file or directory will not be indexed by the operating system's content indexing
     * service.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes NotContentIndexed;

    /**
     * @brief The file or directory is excluded from the data integrity scan. When this value is
     * applied to a directory, by default, all new files and subdirectories within that directory
     * are excluded from data integrity.
     */
    AZ_STORAGE_FILES_SHARES_DLLEXPORT const static FileAttributes NoScrubData;

  private:
    std::vector<std::string> m_value;
  };

}}}}} // namespace Azure::Storage::Files::Shares::Models

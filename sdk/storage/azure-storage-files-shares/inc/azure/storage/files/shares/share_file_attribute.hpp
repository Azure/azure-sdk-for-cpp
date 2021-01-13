// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <set>
#include <string>
#include <type_traits>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {
  class FileAttributes {
  public:
    FileAttributes() = default;
    explicit FileAttributes(std::string value);
    bool operator==(const FileAttributes& other) const;
    bool operator!=(const FileAttributes& other) const;
    FileAttributes operator|(FileAttributes rhs);
    FileAttributes operator&(FileAttributes rhs);
    const std::string& Get();

    /**
     * @brief The File or Directory is read-only.
     */
    const static FileAttributes ReadOnly;

    /**
     * @brief The File or Directory is hidden, and thus is not included in an ordinary directory
     * listing.
     */
    const static FileAttributes Hidden;

    /**
     * @brief The File or Directory is a systemfile.  That is, the file is part of the operating
     * system or is used exclusively by the operating system.
     */
    const static FileAttributes System;

    /**
     * @brief The file  or directory is a standard file that has no special attributes. This
     * attribute is valid only if it is used alone.
     */
    const static FileAttributes None;

    /**
     * @brief The file is a directory.
     */
    const static FileAttributes Directory;

    /**
     * @brief The file is a candidate for backup or removal.
     */
    const static FileAttributes Archive;

    /**
     * @brief The file or directory is temporary. A temporary file contains data that is needed
     * while an application is executing but is not needed after the application is finished. File
     * systems try to keep all the data in memory for quicker access rather than flushing the data
     * back to mass storage. A temporary file should be deleted by the application as soon as it
     * is no longer needed.
     */
    const static FileAttributes Temporary;

    /**
     * @brief The file or directory is offline. The data of the file is not immediately available.
     */
    const static FileAttributes Offline;

    /**
     * @brief The file or directory will not be indexed by the operating system's content indexing
     * service.
     */
    const static FileAttributes NotContentIndexed;

    /**
     * @brief The file or directory is excluded from the data integrity scan. When this value is
     * applied to a directory, by default, all new files and subdirectories within that directory
     * are excluded from data integrity.
     */
    const static FileAttributes NoScrubData;

  private:
    std::set<std::string> m_value;
  };
}}}}} // namespace Azure::Storage::Files::Shares::Models
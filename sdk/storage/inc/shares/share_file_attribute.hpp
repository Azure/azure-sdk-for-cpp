// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  enum class FileAttributes
  {
    /**
     * @brief The File or Directory is read-only.
     */
    ReadOnly = 1,

    /**
     * @brief The File or Directory is hidden, and thus is not included in an ordinary directory
     * listing.
     */
    Hidden = 2,

    /**
     * @brief The File or Directory is a systemfile.  That is, the file is part of the operating
     * system or is used exclusively by the operating system.
     */
    System = 4,

    /**
     * @brief The file  or directory is a standard file that has no special attributes. This
     * attribute is valid only if it is used alone.
     */
    None = 8,

    /**
     * @brief The file is a directory.
     */
    Directory = 16,

    /**
     * @brief The file is a candidate for backup or removal.
     */
    Archive = 32,

    /**
     * @brief The file or directory is temporary. A temporary file contains data that is needed
     * while an application is executing but is not needed after the application is finished. File
     * systems try to keep all the data in memory for quicker access rather than flushing the data
     * back to mass storage. A temporary file should be deleted by the application as soon as it is
     * no longer needed.
     */
    Temporary = 64,

    /**
     * @brief The file or directory is offline. The data of the file is not immediately available.
     */
    Offline = 128,

    /**
     * @brief The file or directory will not be indexed by the operating system's content indexing
     * service.
     */
    NotContentIndexed = 256,

    /**
     * @brief The file or directory is excluded from the data integrity scan. When this value is
     * applied to a directory, by default, all new files and subdirectories within that directory
     * are excluded from data integrity.
     */
    NoScrubData = 512
  };

  inline FileAttributes operator|(FileAttributes lhs, FileAttributes rhs)
  {
    using type = std::underlying_type_t<FileAttributes>;
    return static_cast<FileAttributes>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline FileAttributes& operator|=(FileAttributes& lhs, FileAttributes rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  inline FileAttributes operator&(FileAttributes lhs, FileAttributes rhs)
  {
    using type = std::underlying_type_t<FileAttributes>;
    return static_cast<FileAttributes>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  inline FileAttributes& operator&=(FileAttributes& lhs, FileAttributes rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  inline FileAttributes FileAttributesFromString(const std::string& fileAttributesString)
  {
    FileAttributes result = static_cast<FileAttributes>(0);

    if (fileAttributesString == "ReadOnly")
    {
      result = FileAttributes::ReadOnly;
    }
    else if (fileAttributesString == "Hidden")
    {
      result = FileAttributes::Hidden;
    }
    else if (fileAttributesString == "System")
    {
      result = FileAttributes::System;
    }
    else if (fileAttributesString == "None")
    {
      result = FileAttributes::None;
    }
    else if (fileAttributesString == "Directory")
    {
      result = FileAttributes::Directory;
    }
    else if (fileAttributesString == "Archive")
    {
      result = FileAttributes::Archive;
    }
    else if (fileAttributesString == "Offline")
    {
      result = FileAttributes::Offline;
    }
    else if (fileAttributesString == "NotContentIndexed")
    {
      result = FileAttributes::NotContentIndexed;
    }
    else if (fileAttributesString == "NoScrubData")
    {
      result = FileAttributes::NoScrubData;
    }

    return result;
  }

  inline std::string FileAttributesToString(const FileAttributes& val)
  {
    FileAttributes value_list[] = {
        FileAttributes::ReadOnly,
        FileAttributes::Hidden,
        FileAttributes::System,
        FileAttributes::None,
        FileAttributes::Directory,
        FileAttributes::Archive,
        FileAttributes::Temporary,
        FileAttributes::Offline,
        FileAttributes::NotContentIndexed,
        FileAttributes::NoScrubData,
    };
    const char* string_list[] = {
        "ReadOnly",
        "Hidden",
        "System",
        "None",
        "Directory",
        "Archive",
        "Temporary",
        "Offline",
        "NotContentIndexed",
        "NoScrubData",
    };
    std::string result;
    for (std::size_t i = 0; i < sizeof(value_list) / sizeof(ListSharesIncludeType); ++i)
    {
      if ((val & value_list[i]) == value_list[i])
      {
        if (!result.empty())
        {
          result += "|";
        }
        result += string_list[i];
      }
    }
    return result;
  }

  inline FileAttributes FileAttributesListFromString(const std::string& fileAttributesString)
  {
    FileAttributes results = static_cast<FileAttributes>(0);

    std::string::const_iterator cur = fileAttributesString.begin();

    auto getSubstrTillDelimiter
        = [](char delimiter, const std::string& string, std::string::const_iterator& cur) {
            auto begin = cur;
            auto end = std::find(cur, string.end(), delimiter);
            cur = end;
            if (cur != string.end())
            {
              ++cur;
            }
            return std::string(begin, end);
          };

    while (cur != fileAttributesString.end())
    {
      std::string attribute = getSubstrTillDelimiter('|', fileAttributesString, cur);

      if (!attribute.empty())
      {
        results |= FileAttributesFromString(attribute);
      }
    }

    return results;
  }

}}}} // namespace Azure::Storage::Files::Shares
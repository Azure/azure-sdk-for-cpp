// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/core/datetime.hpp>
#include <azure/storage/blobs/blob_options.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  Azure::Core::Url GetBlobUrlFromUrl(const Azure::Core::Url& url);
  Azure::Core::Url GetDfsUrlFromUrl(const Azure::Core::Url& url);
  std::string GetBlobUrlFromUrl(const std::string& url);
  std::string GetDfsUrlFromUrl(const std::string& url);

  std::string SerializeMetadata(const Storage::Metadata& dataLakePropertiesMap);

  std::string GetSubstringTillDelimiter(
      char delimiter,
      const std::string& string,
      std::string::const_iterator& cur);

  bool MetadataIncidatesIsDirectory(const Storage::Metadata& metadata);

  Blobs::BlobClientOptions GetBlobClientOptions(const DataLakeClientOptions& options);

  /**
   * @brief Provides conversion methods for Win32 FILETIME to an #Azure::DateTime.
   *
   */
  class Win32FileTimeConverter final {
  public:
    /**
     * @brief Converts Win32 FILETIME to an #Azure::DateTime.
     *
     * @param win32Filetime The number of 100-nanoseconds since 1601-01-01.
     * @return Calculated #Azure::DateTime.
     */
    static DateTime Win32FileTimeToDateTime(int64_t win32Filetime)
    {
      auto t = DateTime(1601) + Azure::_detail::Clock::duration(win32Filetime);
      return DateTime(t);
    }

    /**
     * @brief Converts a DateTime to Win32 FILETIME.
     *
     * @param dateTime The `%DateTime` to convert.
     * @return The number of 100-nanoseconds since 1601-01-01.
     */
    static int64_t DateTimeToWin32FileTime(DateTime const& dateTime)
    {
      return std::chrono::duration_cast<Azure::_detail::Clock::duration>(dateTime - DateTime(1601))
          .count();
    }

    /**
     * @brief An instance of `%Win32FileTimeConverter` class cannot be created.
     *
     */
    Win32FileTimeConverter() = delete;

    /**
     * @brief An instance of `%Win32FileTimeConverter` class cannot be destructed, because no
     * instance can be created.
     *
     */
    ~Win32FileTimeConverter() = delete;
  };

}}}}} // namespace Azure::Storage::Files::DataLake::_detail

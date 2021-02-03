// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {

  std::string GetBlobUrlFromUrl(const std::string& url);
  std::string GetDfsUrlFromUrl(const std::string& url);

  std::string SerializeMetadata(const Storage::Metadata& dataLakePropertiesMap);

  std::string GetSubstringTillDelimiter(
      char delimiter,
      const std::string& string,
      std::string::const_iterator& cur);

  bool MetadataIncidatesIsDirectory(const Storage::Metadata& metadata);

}}}}} // namespace Azure::Storage::Files::DataLake::Details

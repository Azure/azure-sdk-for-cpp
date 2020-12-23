// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {

  std::string GetBlobUriFromUri(const std::string& uri);
  std::string GetDfsUriFromUri(const std::string& uri);

  std::string SerializeMetadata(const Storage::Metadata& dataLakePropertiesMap);

  std::string GetSubstringTillDelimiter(
      char delimiter,
      const std::string& string,
      std::string::const_iterator& cur);

}}}}} // namespace Azure::Storage::Files::DataLake::Details

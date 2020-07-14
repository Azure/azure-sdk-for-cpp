// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_uri_builder.hpp"

#include <map>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {
  UriBuilder GetBlobUriFromDfsUri(const UriBuilder& dfsUri);
  UriBuilder GetDfsUriFromBlobUri(const UriBuilder& blobUri);

  void InitializeUrisFromServiceUri(
      const std::string& serviceUriString,
      UriBuilder& dfsUri,
      UriBuilder& blobUri);

  std::map<std::string, std::string> DeserializeMetadata(
      const std::string& dataLakePropertiesString);

  std::string SerializeMetadata(const std::map<std::string, std::string>& dataLakePropertiesMap);

  std::string GetSubstringTillDelimiter(
      char delimiter,
      const std::string& string,
      std::string::const_iterator& cur);

}}}}} // namespace Azure::Storage::Files::DataLake::Details

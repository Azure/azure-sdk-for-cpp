// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_uri_builder.hpp"

#include <map>
#include <string>

namespace Azure { namespace Storage { namespace DataLake { namespace Details {
  UriBuilder GetBlobUriFromDfsUri(const UriBuilder& dfsUri);

  std::map<std::string, std::string> DeserializeMetadata(
      const std::string& dataLakePropertiesString);

  std::string SerializeMetadata(const std::map<std::string, std::string>& dataLakePropertiesMap);

}}}} // namespace Azure::Storage::DataLake::Details

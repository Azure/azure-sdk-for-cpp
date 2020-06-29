// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_url_builder.hpp"
#include "nullable.hpp"

#include <map>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace DataLake { namespace Details {
  UrlBuilder GetBlobUriFromDfsUri(const UrlBuilder& dfsUri);

  Azure::Core::Nullable<std::map<std::string, std::string>> DeserializeMetadata(
      const Azure::Core::Nullable<std::string>& dataLakePropertiesString);

  Azure::Core::Nullable<std::string> SerializeMetadata(
      const Azure::Core::Nullable<std::map<std::string, std::string>>& dataLakePropertiesMap);

}}}} // namespace Azure::Storage::DataLake::Details

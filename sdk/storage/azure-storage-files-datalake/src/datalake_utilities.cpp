// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_utilities.hpp"

#include <azure/storage/common/crypt.hpp>

#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {

  const static std::string c_DfsEndPointIdentifier = ".dfs.";
  const static std::string c_BlobEndPointIdentifier = ".blob.";

  std::string GetBlobUriFromUri(const std::string& uri)
  {
    std::string result = uri;
    auto pos = result.find(c_DfsEndPointIdentifier);
    if (pos != std::string::npos)
    {
      result.replace(pos, c_DfsEndPointIdentifier.size(), c_BlobEndPointIdentifier);
    }
    return result;
  }

  std::string GetDfsUriFromUri(const std::string& uri)
  {
    std::string result = uri;
    auto pos = result.find(c_BlobEndPointIdentifier);
    if (pos != std::string::npos)
    {
      result.replace(pos, c_BlobEndPointIdentifier.size(), c_DfsEndPointIdentifier);
    }
    return result;
  }

  std::string SerializeMetadata(const Storage::Metadata& dataLakePropertiesMap)
  {
    std::string result;
    for (const auto& pair : dataLakePropertiesMap)
    {
      result.append(
          pair.first + "="
          + Azure::Core::Base64Encode(std::vector<uint8_t>(pair.second.begin(), pair.second.end()))
          + ",");
    }
    if (!result.empty())
    {
      result.pop_back();
    }
    return result;
  }

  std::string GetSubstringTillDelimiter(
      char delimiter,
      const std::string& string,
      std::string::const_iterator& cur)
  {
    auto begin = cur;
    auto end = std::find(cur, string.end(), delimiter);
    cur = end;
    if (cur != string.end())
    {
      ++cur;
    }
    return std::string(begin, end);
  }
}}}}} // namespace Azure::Storage::Files::DataLake::Details

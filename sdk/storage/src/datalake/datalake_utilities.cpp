// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake_utilities.hpp"

#include "common/crypt.hpp"
#include "datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {
  std::string GetBlobUriFromUri(const std::string& uri)
  {
    std::string result = uri;
    auto pos = result.find(".dfs.");
    if (pos != std::string::npos)
    {
      result.replace(pos, 6u, std::string(".blob."));
    }
    return result;
  }

  std::string GetDfsUriFromUri(const std::string& uri)
  {
    std::string result = uri;
    auto pos = result.find(".blob.");
    if (pos != std::string::npos)
    {
      result.replace(pos, 5u, std::string(".dfs."));
    }
    // DfsUri will be empty if there is no dfs endpoint.
    pos = result.find(".dfs.");
    if (pos == std::string::npos)
    {
      result.clear();
    }
    return result;
  }

  std::map<std::string, std::string> DeserializeMetadata(
      const std::string& dataLakePropertiesString)
  {
    std::map<std::string, std::string> result;

    std::string::const_iterator cur = dataLakePropertiesString.begin();

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

    while (cur != dataLakePropertiesString.end())
    {
      std::string key = getSubstrTillDelimiter('=', dataLakePropertiesString, cur);
      std::string val = getSubstrTillDelimiter(',', dataLakePropertiesString, cur);

      if (!key.empty() || !val.empty())
      {
        result[std::move(key)] = Base64Decode(val);
      }
    }

    return result;
  }

  std::string SerializeMetadata(const std::map<std::string, std::string>& dataLakePropertiesMap)
  {
    std::string result;
    for (const auto& pair : dataLakePropertiesMap)
    {
      result.append(pair.first + "=" + Base64Encode(pair.second) + ",");
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

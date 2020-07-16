// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake_utilities.hpp"

#include "common/crypt.hpp"
#include "datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace DataLake { namespace Details {
  UriBuilder GetBlobUriFromDfsUri(const UriBuilder& dfsUri)
  {
    UriBuilder result = dfsUri;
    auto hoststr = result.GetHost();
    auto pos = hoststr.find(".dfs.");
    if (pos != std::string::npos)
    {
      result.SetHost(hoststr.replace(pos, 5u, std::string(".blob.")));
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
}}}} // namespace Azure::Storage::DataLake::Details

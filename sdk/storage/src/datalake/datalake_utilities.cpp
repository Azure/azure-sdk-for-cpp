// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake_utilities.hpp"

#include "common/crypt.hpp"
#include "datalake/protocol/datalake_rest_client.hpp"

#include <regex>

namespace Azure { namespace Storage { namespace DataLake {
  namespace Details {
    UrlBuilder GetBlobUriFromDfsUri(const UrlBuilder& dfsUri)
    {
      UrlBuilder result = dfsUri;
      if (std::regex_match(
              dfsUri.GetHost(), std::regex(std::string(".*") + Details::c_PathDnsSuffixDefault)))
      {
        result.SetHost(std::regex_replace(result.GetHost(), std::regex(".dfs."), ".blob."));
      }

      return result;
    }
  } // namespace Details
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

  inline std::string SerializeMetadata(
      const std::map<std::string, std::string>& dataLakePropertiesMap)
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
}}} // namespace Azure::Storage::DataLake

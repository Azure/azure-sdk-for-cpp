// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/datalake_utilities.hpp"

#include <azure/storage/common/crypt.hpp>

#include "private/datalake_constants.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  const static std::string DfsEndPointIdentifier = ".dfs.";
  const static std::string BlobEndPointIdentifier = ".blob.";

  Azure::Core::Url GetBlobUrlFromUrl(const Azure::Core::Url& url)
  {
    std::string host = url.GetHost();
    auto pos = host.rfind(DfsEndPointIdentifier);
    if (pos == std::string::npos)
    {
      return url;
    }
    host.replace(pos, DfsEndPointIdentifier.size(), BlobEndPointIdentifier);
    Azure::Core::Url result = url;
    result.SetHost(host);
    return result;
  }

  Azure::Core::Url GetDfsUrlFromUrl(const Azure::Core::Url& url)
  {
    std::string host = url.GetHost();
    auto pos = host.rfind(BlobEndPointIdentifier);
    if (pos == std::string::npos)
    {
      return url;
    }
    host.replace(pos, BlobEndPointIdentifier.size(), DfsEndPointIdentifier);
    Azure::Core::Url result = url;
    result.SetHost(host);
    return result;
  }

  std::string GetBlobUrlFromUrl(const std::string& url)
  {
    return GetBlobUrlFromUrl(Azure::Core::Url(url)).GetAbsoluteUrl();
  }

  std::string GetDfsUrlFromUrl(const std::string& url)
  {
    return GetDfsUrlFromUrl(Azure::Core::Url(url)).GetAbsoluteUrl();
  }

  std::string SerializeMetadata(const Storage::Metadata& dataLakePropertiesMap)
  {
    std::string result;
    for (const auto& pair : dataLakePropertiesMap)
    {
      result.append(
          pair.first + "="
          + Azure::Core::Convert::Base64Encode(
              std::vector<uint8_t>(pair.second.begin(), pair.second.end()))
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

  bool MetadataIncidatesIsDirectory(const Storage::Metadata& metadata)
  {
    auto ite = metadata.find(DataLakeIsDirectoryKey);
    return ite != metadata.end() && ite->second == "true";
  }

  Blobs::BlobClientOptions GetBlobClientOptions(const DataLakeClientOptions& options)
  {
    Blobs::BlobClientOptions blobOptions;
    *(static_cast<Azure::Core::_internal::ClientOptions*>(&blobOptions)) = options;
    blobOptions.SecondaryHostForRetryReads
        = _detail::GetBlobUrlFromUrl(options.SecondaryHostForRetryReads);
    blobOptions.ApiVersion = options.ApiVersion;
    return blobOptions;
  }

}}}}} // namespace Azure::Storage::Files::DataLake::_detail

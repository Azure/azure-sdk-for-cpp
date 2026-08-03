// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/datalake_utilities.hpp"

#include "private/datalake_constants.hpp"

#include <azure/storage/common/crypt.hpp>

#include <array>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  const static std::string DfsEndPointIdentifier = ".dfs.";
  const static std::string BlobEndPointIdentifier = ".blob.";

  namespace {
    bool EndsWith(const std::string& value, const std::string& suffix)
    {
      return value.size() >= suffix.size()
          && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    std::string ToLowerAscii(std::string value)
    {
      for (auto& character : value)
      {
        if (character >= 'A' && character <= 'Z')
        {
          character = static_cast<char>(character - 'A' + 'a');
        }
      }
      return value;
    }

    int HexDigitValue(char value)
    {
      if (value >= '0' && value <= '9')
      {
        return value - '0';
      }
      if (value >= 'A' && value <= 'F')
      {
        return value - 'A' + 10;
      }
      if (value >= 'a' && value <= 'f')
      {
        return value - 'a' + 10;
      }
      return -1;
    }

    std::string CompactWorkspaceId(const std::string& workspaceId)
    {
      if (workspaceId.size() != 32 && workspaceId.size() != 36)
      {
        throw std::invalid_argument("OneLake workspace ID must be a GUID");
      }

      std::string compact;
      compact.reserve(32);
      for (std::size_t index = 0; index < workspaceId.size(); ++index)
      {
        if (workspaceId.size() == 36 && (index == 8 || index == 13 || index == 18 || index == 23))
        {
          if (workspaceId[index] != '-')
          {
            throw std::invalid_argument("OneLake workspace ID must be a GUID");
          }
          continue;
        }

        if (HexDigitValue(workspaceId[index]) < 0)
        {
          throw std::invalid_argument("OneLake workspace ID must be a GUID");
        }
        compact.push_back(ToLowerAscii(workspaceId.substr(index, 1))[0]);
      }
      return compact;
    }

    std::vector<std::string> SplitLabels(const std::string& value)
    {
      std::vector<std::string> labels;
      std::size_t begin = 0;
      while (begin <= value.size())
      {
        const auto end = value.find('.', begin);
        labels.emplace_back(value.substr(begin, end - begin));
        if (end == std::string::npos)
        {
          break;
        }
        begin = end + 1;
      }
      return labels;
    }

    void ValidateDnsName(const std::string& host)
    {
      if (host.empty() || host.size() > 253)
      {
        throw std::invalid_argument("Unsupported OneLake service URL");
      }
      for (const auto& label : SplitLabels(host))
      {
        if (label.empty() || label.size() > 63)
        {
          throw std::invalid_argument("Unsupported OneLake service URL");
        }
      }
    }

    bool IsValidRegionPrefix(const std::string& prefix)
    {
      if (prefix.empty() || prefix.front() == '-' || prefix.back() == '-')
      {
        return false;
      }

      bool previousWasHyphen = false;
      for (const auto character : prefix)
      {
        const bool isHyphen = character == '-';
        if ((!isHyphen && !(character >= 'a' && character <= 'z')
             && !(character >= '0' && character <= '9'))
            || (isHyphen && previousWasHyphen))
        {
          return false;
        }
        previousWasHyphen = isHyphen;
      }
      return true;
    }

    std::string GetRingFromPrefix(const std::string& prefix)
    {
      for (const auto& ring : std::array<std::string, 3>{"daily", "dxt", "msit"})
      {
        for (const auto& alias : std::array<std::string, 2>{ring, "i-" + ring})
        {
          if (prefix == alias)
          {
            return ring + "-";
          }
          const std::string ringPrefix = alias + "-";
          if (prefix.compare(0, ringPrefix.size(), ringPrefix) == 0
              && IsValidRegionPrefix(prefix.substr(ringPrefix.size())))
          {
            return ring + "-";
          }
        }
      }
      if (IsValidRegionPrefix(prefix))
      {
        return {};
      }
      throw std::invalid_argument("Unsupported OneLake service URL");
    }

    std::string GetSharedRing(const std::string& serviceLabel, const std::string& baseLabel)
    {
      if (serviceLabel == baseLabel)
      {
        return {};
      }

      const std::string separatorAndBaseLabel = "-" + baseLabel;
      if (!EndsWith(serviceLabel, separatorAndBaseLabel))
      {
        throw std::invalid_argument("Unsupported OneLake service URL");
      }
      return GetRingFromPrefix(
          serviceLabel.substr(0, serviceLabel.size() - separatorAndBaseLabel.size()));
    }

    std::string GetWorkspaceRing(const std::string& endpointLabel)
    {
      if (endpointLabel == "dfs" || endpointLabel == "blob")
      {
        return {};
      }
      for (const auto& ring : std::array<std::string, 3>{"daily", "dxt", "msit"})
      {
        if (endpointLabel == ring + "-dfs" || endpointLabel == ring + "-blob")
        {
          return ring + "-";
        }
      }
      throw std::invalid_argument("Unsupported OneLake service URL");
    }

    struct ParsedOneLakeHost final
    {
      std::string CloudDomain;
      std::string Ring;
    };

    ParsedOneLakeHost ParseOneLakeHost(
        const std::string& host,
        const std::string& compactWorkspaceId)
    {
      std::string cloudDomain;
      std::string relativeHost;
      for (const auto& supportedDomain : std::array<std::string, 4>{
               "fabric.microsoft.com",
               "fabric-df.microsoft.com",
               "fabric.microsoft.us",
               "fabric.sovcloud-api.fr"})
      {
        const std::string cloudSuffix = "." + supportedDomain;
        if (EndsWith(host, cloudSuffix))
        {
          cloudDomain = supportedDomain;
          relativeHost = host.substr(0, host.size() - cloudSuffix.size());
          break;
        }
      }
      if (cloudDomain.empty())
      {
        throw std::invalid_argument("Unsupported OneLake service URL");
      }

      const auto labels = SplitLabels(relativeHost);
      if (labels.size() == 2 && (labels[1] == "dfs" || labels[1] == "blob"))
      {
        return ParsedOneLakeHost{cloudDomain, GetSharedRing(labels[0], "onelake")};
      }
      if (labels.size() == 2 && labels[1] == "onelake")
      {
        return ParsedOneLakeHost{cloudDomain, GetSharedRing(labels[0], "api")};
      }
      if (labels.size() == 3 && labels[0] == compactWorkspaceId
          && labels[1] == "z" + compactWorkspaceId.substr(0, 2))
      {
        return ParsedOneLakeHost{cloudDomain, GetWorkspaceRing(labels[2])};
      }
      throw std::invalid_argument("Unsupported OneLake service URL");
    }
  } // namespace

  OneLakeWorkspaceEndpoints GetOneLakeWorkspaceEndpoints(
      const std::string& oneLakeServiceUrl,
      const std::string& workspaceId)
  {
    if (oneLakeServiceUrl.find_first_of("@?#") != std::string::npos)
    {
      throw std::invalid_argument("Unsupported OneLake service URL");
    }
    const auto authorityBegin = oneLakeServiceUrl.find("://");
    if (authorityBegin == std::string::npos)
    {
      throw std::invalid_argument("Unsupported OneLake service URL");
    }
    const auto authorityEnd = oneLakeServiceUrl.find('/', authorityBegin + 3);
    if (oneLakeServiceUrl.find(':', authorityBegin + 3) < authorityEnd)
    {
      throw std::invalid_argument("Unsupported OneLake service URL");
    }
    if (authorityEnd != std::string::npos && oneLakeServiceUrl.substr(authorityEnd) != "/")
    {
      throw std::invalid_argument("Unsupported OneLake service URL");
    }

    Azure::Core::Url serviceUrl(oneLakeServiceUrl);
    if (ToLowerAscii(serviceUrl.GetScheme()) != "https"
        || (!serviceUrl.GetPath().empty() && serviceUrl.GetPath() != "/")
        || serviceUrl.GetPort() != 0)
    {
      throw std::invalid_argument("Unsupported OneLake service URL");
    }

    const std::string compactWorkspaceId = CompactWorkspaceId(workspaceId);
    const auto normalizedHost = ToLowerAscii(serviceUrl.GetHost());
    ValidateDnsName(normalizedHost);
    const auto parsedHost = ParseOneLakeHost(normalizedHost, compactWorkspaceId);
    const std::string workspacePrefix = compactWorkspaceId + ".z" + compactWorkspaceId.substr(0, 2);

    Azure::Core::Url dfsUrl(
        "https://" + workspacePrefix + "." + parsedHost.Ring + "dfs." + parsedHost.CloudDomain);
    Azure::Core::Url blobUrl(
        "https://" + workspacePrefix + "." + parsedHost.Ring + "blob." + parsedHost.CloudDomain);
    const std::string encodedWorkspaceId = Azure::Storage::_internal::UrlEncodePath(workspaceId);
    dfsUrl.AppendPath(encodedWorkspaceId);
    blobUrl.AppendPath(encodedWorkspaceId);
    return OneLakeWorkspaceEndpoints{std::move(dfsUrl), std::move(blobUrl)};
  }

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

  bool MetadataIndicatesIsDirectory(const Storage::Metadata& metadata)
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
    blobOptions.CustomerProvidedKey = options.CustomerProvidedKey;
    blobOptions.EnableTenantDiscovery = options.EnableTenantDiscovery;
    if (options.Audience.HasValue())
    {
      blobOptions.Audience = Blobs::BlobAudience(options.Audience.Value().ToString());
    }
    if (options.DownloadValidationOptions.HasValue())
    {
      Blobs::TransferValidationOptions validationOptions;
      validationOptions.Algorithm = options.DownloadValidationOptions.Value().Algorithm;
      blobOptions.DownloadValidationOptions = std::move(validationOptions);
    }
    if (options.UploadValidationOptions.HasValue())
    {
      Blobs::TransferValidationOptions validationOptions;
      validationOptions.Algorithm = options.UploadValidationOptions.Value().Algorithm;
      blobOptions.UploadValidationOptions = std::move(validationOptions);
    }
    return blobOptions;
  }

}}}}} // namespace Azure::Storage::Files::DataLake::_detail

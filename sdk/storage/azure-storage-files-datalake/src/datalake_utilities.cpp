// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/datalake_utilities.hpp"

#include "private/datalake_constants.hpp"

#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  const static std::string DfsEndPointIdentifier = ".dfs.";
  const static std::string BlobEndPointIdentifier = ".blob.";

  Models::PathProperties PathPropertiesFromBlobProperties(
      Blobs::Models::BlobProperties properties,
      const Azure::Core::Http::RawResponse& response)
  {
    Models::PathProperties ret;
    ret.ETag = std::move(properties.ETag);
    ret.LastModified = std::move(properties.LastModified);
    ret.CreatedOn = std::move(properties.CreatedOn);
    ret.Metadata = std::move(properties.Metadata);
    ret.LeaseDuration = std::move(properties.LeaseDuration);
    ret.LeaseState = std::move(properties.LeaseState);
    ret.LeaseStatus = std::move(properties.LeaseStatus);
    ret.HttpHeaders = std::move(properties.HttpHeaders);
    ret.IsServerEncrypted = properties.IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(properties.EncryptionKeySha256);
    ret.EncryptionScope = std::move(properties.EncryptionScope);
    ret.CopyId = std::move(properties.CopyId);
    ret.CopySource = std::move(properties.CopySource);
    ret.CopyStatus = std::move(properties.CopyStatus);
    ret.CopyProgress = std::move(properties.CopyProgress);
    ret.CopyCompletedOn = std::move(properties.CopyCompletedOn);
    ret.ExpiresOn = std::move(properties.ExpiresOn);
    ret.LastAccessedOn = std::move(properties.LastAccessedOn);
    ret.FileSize = properties.BlobSize;
    ret.ArchiveStatus = std::move(properties.ArchiveStatus);
    ret.RehydratePriority = std::move(properties.RehydratePriority);
    ret.CopyStatusDescription = std::move(properties.CopyStatusDescription);
    ret.IsIncrementalCopy = std::move(properties.IsIncrementalCopy);
    ret.IncrementalCopyDestinationSnapshot
        = std::move(properties.IncrementalCopyDestinationSnapshot);
    ret.VersionId = std::move(properties.VersionId);
    ret.IsCurrentVersion = std::move(properties.IsCurrentVersion);
    ret.IsDirectory = MetadataIndicatesIsDirectory(ret.Metadata);
    const auto& headers = response.GetHeaders();
    auto encryptionContext = headers.find(EncryptionContextHeaderName);
    if (encryptionContext != headers.end())
    {
      ret.EncryptionContext = encryptionContext->second;
    }
    auto acl = headers.find(AclHeaderName);
    if (acl != headers.end())
    {
      ret.Acls = Models::Acl::DeserializeAcls(acl->second);
    }
    auto owner = headers.find(OwnerHeaderName);
    if (owner != headers.end())
    {
      ret.Owner = owner->second;
    }
    auto group = headers.find(GroupHeaderName);
    if (group != headers.end())
    {
      ret.Group = group->second;
    }
    auto permissions = headers.find(PermissionsHeaderName);
    if (permissions != headers.end())
    {
      ret.Permissions = permissions->second;
    }
    return ret;
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
    blobOptions.Session = options.Session;
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

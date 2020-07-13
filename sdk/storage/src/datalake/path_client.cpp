// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/path_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake/datalake_utilities.hpp"
#include "http/curl/curl.hpp"

#include <limits>
#include <utility> //std::pair

namespace Azure { namespace Storage { namespace DataLake {

  namespace {
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

    std::pair<int64_t, int64_t> GetOffsetLength(const std::string& rangeString)
    {
      int64_t offset = std::numeric_limits<int64_t>::max();
      int64_t length = std::numeric_limits<int64_t>::max();
      const std::string c_bytesPrefix = "bytes=";
      if (rangeString.length() > c_bytesPrefix.length())
      {
        auto subRangeString = rangeString.substr(c_bytesPrefix.length());
        std::string::const_iterator cur = subRangeString.begin();
        offset = std::stoll(GetSubstringTillDelimiter('=', subRangeString, cur));
        if (cur != subRangeString.end())
        {
          length = std::stoll(GetSubstringTillDelimiter('\n', subRangeString, cur)) - offset + 1;
        }
        else
        {
          throw std::runtime_error("The format of the range string is not correct: " + rangeString);
        }
      }
      return std::make_pair(offset, length);
    }
  } // namespace

  Acl Acl::FromString(const std::string& aclString)
  {
    std::string::const_iterator cur = aclString.begin();
    std::string scope = GetSubstringTillDelimiter(':', aclString, cur);
    std::string type = GetSubstringTillDelimiter(':', aclString, cur);
    std::string id = GetSubstringTillDelimiter(':', aclString, cur);
    std::string permissions = GetSubstringTillDelimiter(':', aclString, cur);

    return Acl{std::move(scope), std::move(type), std::move(id), std::move(permissions)};
  }

  std::string Acl::ToString(const Acl& acl)
  {
    std::string result;
    result = acl.Scope + ":" + acl.Type + ":" + acl.Id + ":" + acl.Permissions;
    return result;
  }

  std::vector<Acl> Acl::DeserializeAcls(const std::string& dataLakeAclsString)
  {
    std::vector<Acl> result;

    std::string::const_iterator cur = dataLakeAclsString.begin();

    while (cur != dataLakeAclsString.end())
    {
      result.emplace_back(FromString(GetSubstringTillDelimiter(',', dataLakeAclsString, cur)));
    }

    return result;
  }
  std::string Acl::SerializeAcls(const std::vector<Acl>& dataLakeAclArray)
  {
    std::string result;
    for (const auto& acl : dataLakeAclArray)
    {
      result.append(ToString(acl) + ",");
    }
    if (!result.empty())
    {
      result.pop_back();
    }
    return result;
  }

  PathClient PathClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& path,
      const PathClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto pathUri = std::move(parsedConnectionString.DataLakeServiceUri);
    pathUri.AppendPath(fileSystemName, true);
    pathUri.AppendPath(path, true);

    if (parsedConnectionString.KeyCredential)
    {
      return PathClient(pathUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return PathClient(pathUri.ToString(), options);
    }
  }

  PathClient::PathClient(
      const std::string& pathUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const PathClientOptions& options)
      : m_dfsUri(pathUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathClient::PathClient(
      const std::string& pathUri,
      std::shared_ptr<TokenCredential> credential,
      const PathClientOptions& options)
      : m_dfsUri(pathUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<TokenCredentialPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathClient::PathClient(const std::string& pathUri, const PathClientOptions& options)
      : m_dfsUri(pathUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathAppendDataResponse PathClient::AppendData(
      std::unique_ptr<Azure::Core::Http::BodyStream> stream,
      int64_t offset,
      const PathAppendDataOptions& options) const
  {
    DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = stream->Length();
    protocolLayerOptions.TransactionalContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::AppendData(
        m_dfsUri.ToString(), *stream, *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathFlushDataResponse PathClient::FlushData(int64_t offset, const PathFlushDataOptions& options)
      const
  {
    DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.CacheControl = options.HttpHeaders.CacheControl;
    protocolLayerOptions.ContentType = options.HttpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = options.HttpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.HttpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.HttpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::FlushData(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathSetAccessControlResponse PathClient::SetAccessControl(
      std::vector<Acl> acls,
      const SetAccessControlOptions& options) const
  {
    DataLakeRestClient::Path::SetAccessControlOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Acl = Acl::SerializeAcls(acls);
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::SetAccessControl(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathSetAccessControlRecursiveResponse PathClient::SetAccessControlRecursive(
      PathSetAccessControlRecursiveMode mode,
      std::vector<Acl> acls,
      const SetAccessControlRecursiveOptions& options) const
  {
    DataLakeRestClient::Path::SetAccessControlRecursiveOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Mode = mode;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxRecords = options.MaxRecords;
    protocolLayerOptions.Acl = Acl::SerializeAcls(acls);
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::SetAccessControlRecursive(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  SetPathHttpHeadersResponse PathClient::SetHttpHeaders(
      const SetPathHttpHeadersOptions& options) const
  {
    DataLakeRestClient::Path::UpdateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = PathUpdateAction::SetProperties;
    protocolLayerOptions.CacheControl = options.HttpHeaders.CacheControl;
    protocolLayerOptions.ContentType = options.HttpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = options.HttpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.HttpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.HttpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto emptyStream = Azure::Core::Http::NullBodyStream();
    auto result = DataLakeRestClient::Path::Update(
        m_dfsUri.ToString(), emptyStream, *m_pipeline, options.Context, protocolLayerOptions);
    auto returnVal = SetPathHttpHeadersResponse{};
    returnVal.Date = std::move(result.Date);
    returnVal.RequestId = std::move(result.RequestId);
    returnVal.Version = std::move(result.Version);
    returnVal.ETag = std::move(result.ETag);
    returnVal.LastModified = std::move(result.LastModified);
    returnVal.HttpHeaders = std::move(result.HttpHeaders);
    returnVal.ContentLength = result.ContentLength;
    returnVal.ContentRange = std::move(result.ContentRange);
    returnVal.ContentMD5 = std::move(result.ContentMD5);
    auto rawProperties
        = result.Properties.HasValue() ? result.Properties.GetValue() : std::string();
    returnVal.Continuation = std::move(result.Continuation);
    returnVal.DirectoriesSuccessful = result.DirectoriesSuccessful;
    returnVal.FilesSuccessful = result.FilesSuccessful;
    returnVal.FailureCount = result.FailureCount;
    returnVal.FailedEntries = std::move(result.FailedEntries);
    return returnVal;
  }

  PathCreateResponse PathClient::Create(PathResourceType type, const PathCreateOptions& options)
      const
  {
    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Resource = type;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.CacheControl = options.HttpHeaders.CacheControl;
    protocolLayerOptions.ContentType = options.HttpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = options.HttpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.HttpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.HttpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Properties = Details::SerializeMetadata(options.Metadata);
    protocolLayerOptions.Umask = options.Umask;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Create(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathRenameResponse PathClient::Rename(
      const std::string& destinationPath,
      const PathRenameOptions& options)
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_dfsUri.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUri = m_dfsUri;
    destinationDfsUri.SetPath(destinationFileSystem.GetValue() + '/' + destinationPath);

    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.Mode = options.Mode;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RenameSource = "/" + m_dfsUri.GetPath();
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::Create(
        destinationDfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    m_dfsUri = std::move(destinationDfsUri);
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);
    return result;
  }

  PathDeleteResponse PathClient::Delete(const PathDeleteOptions& options) const
  {
    DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RecursiveOptional = options.RecursiveOptional;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Delete(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  GetPathPropertiesResponse PathClient::GetProperties(const PathGetPropertiesOptions& options) const
  {
    DataLakeRestClient::Path::GetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = options.Action;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::GetProperties(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    Azure::Core::Nullable<std::vector<Acl>> acl;
    if (result.ACL.HasValue())
    {
      acl = Acl::DeserializeAcls(result.ACL.GetValue());
    }
    auto returnVal = GetPathPropertiesResponse{};
    returnVal.AcceptRanges = std::move(result.AcceptRanges);
    returnVal.HttpHeaders = std::move(result.HttpHeaders);
    returnVal.ContentLength = result.ContentLength;
    returnVal.ContentMD5 = std::move(result.ContentMD5);
    returnVal.Date = std::move(result.Date);
    returnVal.ETag = std::move(result.ETag);
    returnVal.LastModified = std::move(result.LastModified);
    returnVal.RequestId = std::move(result.RequestId);
    returnVal.Version = std::move(result.Version);
    returnVal.ResourceType = std::move(result.ResourceType);
    returnVal.Owner = std::move(result.Owner);
    returnVal.Group = std::move(result.Group);
    returnVal.Permissions = std::move(result.Permissions);
    returnVal.Acls = std::move(acl);
    returnVal.LeaseDuration = std::move(result.LeaseDuration);
    returnVal.LeaseState = result.LeaseState;
    returnVal.LeaseStatus = result.LeaseStatus;
    auto rawProperties
        = result.Properties.HasValue() ? result.Properties.GetValue() : std::string();
    returnVal.Metadata = Details::DeserializeMetadata(rawProperties);
    return returnVal;
  }

  SetPathMetadataResponse PathClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const SetPathMetadataOptions& options) const
  {
    DataLakeRestClient::Path::UpdateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = PathUpdateAction::SetProperties;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Properties = Details::SerializeMetadata(metadata);
    protocolLayerOptions.Timeout = options.Timeout;
    auto emptyStream = Azure::Core::Http::NullBodyStream();
    auto result = DataLakeRestClient::Path::Update(
        m_dfsUri.ToString(), emptyStream, *m_pipeline, options.Context, protocolLayerOptions);
    auto returnVal = SetPathMetadataResponse{};
    returnVal.Date = std::move(result.Date);
    returnVal.RequestId = std::move(result.RequestId);
    returnVal.Version = std::move(result.Version);
    returnVal.ETag = std::move(result.ETag);
    returnVal.LastModified = std::move(result.LastModified);
    return returnVal;
  }

  std::map<std::string, std::string> PathClient::GetMetadata(
      const GetPathMetadataOptions& options) const
  {
    DataLakeRestClient::Path::GetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = PathGetPropertiesAction::Unknown;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::GetProperties(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    auto rawProperties
        = result.Properties.HasValue() ? result.Properties.GetValue() : std::string();
    return Details::DeserializeMetadata(rawProperties);
  }

  ReadPathResponse PathClient::Read(const PathReadOptions& options) const
  {
    DataLakeRestClient::Path::ReadOptions protocolLayerOptions;
    if (options.Offset.HasValue())
    {
      auto rangeString = std::string("bytes=" + std::to_string(options.Offset.GetValue()) + "-");
      if (options.Length.HasValue())
      {
        rangeString += std::to_string(options.Offset.GetValue() + options.Length.GetValue() - 1);
      }
      protocolLayerOptions.Range = std::move(rangeString);
    }

    protocolLayerOptions.XMsRangeGetContentMd5 = options.RangeGetContentMd5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::Read(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    Azure::Core::Nullable<int64_t> RangeOffset;
    Azure::Core::Nullable<int64_t> RangeLength;
    if (result.ContentRange.HasValue())
    {
      auto range = GetOffsetLength(result.ContentRange.GetValue());
      RangeOffset = range.first;
      RangeLength = range.second;
    }
    auto returnVal = ReadPathResponse{};
    returnVal.Body = std::move(result.BodyStream);
    returnVal.AcceptRanges = std::move(result.AcceptRanges);
    returnVal.HttpHeaders = std::move(result.HttpHeaders);
    returnVal.ContentLength = result.ContentLength;
    returnVal.RangeOffset = RangeOffset;
    returnVal.RangeLength = RangeLength;
    returnVal.TransactionalMD5 = std::move(result.TransactionalMD5);
    returnVal.Date = std::move(result.Date);
    returnVal.ETag = std::move(result.ETag);
    returnVal.LastModified = std::move(result.LastModified);
    returnVal.RequestId = std::move(result.RequestId);
    returnVal.Version = std::move(result.Version);
    returnVal.ResourceType = std::move(result.ResourceType);
    returnVal.LeaseDuration = std::move(result.LeaseDuration);
    returnVal.LeaseState = result.LeaseState;
    returnVal.LeaseStatus = result.LeaseStatus;
    returnVal.ContentMD5 = std::move(result.ContentMD5);
    auto rawProperties
        = result.Properties.HasValue() ? result.Properties.GetValue() : std::string();
    returnVal.Metadata = Details::DeserializeMetadata(rawProperties);
    return returnVal;
  }
}}} // namespace Azure::Storage::DataLake

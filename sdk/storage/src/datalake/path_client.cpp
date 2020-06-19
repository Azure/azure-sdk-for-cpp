// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/path_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/constant.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake/datalake_utilities.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace DataLake {

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
    for (const auto& p : options.policies)
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
    for (const auto& p : options.policies)
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
    for (const auto& p : options.policies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathAppendDataResponse PathClient::AppendData(
      Azure::Core::Http::BodyStream* stream,
      int64_t offset,
      const PathAppendDataOptions& options) const
  {
    DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Body = stream;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = stream->Length();
    protocolLayerOptions.TransactionalContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::AppendData(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
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
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::FlushData(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathSetAccessControlResponse PathClient::SetAccessControl(
      const SetAccessControlOptions& options) const
  {
    DataLakeRestClient::Path::SetAccessControlOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Acl = options.Acl;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::SetAccessControl(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathSetAccessControlRecursiveResponse PathClient::SetAccessControlRecursive(
      PathSetAccessControlRecursiveMode mode,
      const SetAccessControlRecursiveOptions& options) const
  {
    DataLakeRestClient::Path::SetAccessControlRecursiveOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Mode = mode;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxRecords = options.MaxRecords;
    protocolLayerOptions.Acl = options.Acl;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::SetAccessControlRecursive(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathUpdateResponse PathClient::SetProperties(const SetPathPropertiesOptions& options) const
  {
    DataLakeRestClient::Path::UpdateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = PathUpdateAction::SetProperties;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Properties = SerializeMetadata(options.Metadata);
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Update(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathCreateResponse PathClient::Create(const PathCreateOptions& options) const
  {
    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Resource = options.Resource;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Properties = SerializeMetadata(options.Metadata);
    protocolLayerOptions.Umask = options.Umask;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Create(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathRenameResponse PathClient::Rename(const PathRenameOptions& options) const
  {
    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Resource = options.Resource;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.Mode = options.Mode;
    protocolLayerOptions.SourceLeaseId = options.SourceLeaseId;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceIfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceIfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceIfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    protocolLayerOptions.Properties = SerializeMetadata(options.Metadata);
    protocolLayerOptions.Umask = options.Umask;
    protocolLayerOptions.RenameSource = options.RenameSource;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Create(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  PathDeleteResponse PathClient::Delete(const PathDeleteOptions& options) const
  {
    DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
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
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::GetProperties(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    return GetPathPropertiesResponse{
        std::move(result.AcceptRanges),
        std::move(result.CacheControl),
        std::move(result.ContentDisposition),
        std::move(result.ContentEncoding),
        std::move(result.ContentLanguage),
        result.ContentLength,
        std::move(result.ContentRange),
        std::move(result.ContentType),
        std::move(result.ContentMD5),
        std::move(result.Date),
        std::move(result.ETag),
        std::move(result.LastModified),
        std::move(result.RequestId),
        std::move(result.Version),
        std::move(result.ResourceType),
        std::move(result.Owner),
        std::move(result.Group),
        std::move(result.Permissions),
        std::move(result.ACL),
        std::move(result.LeaseDuration),
        std::move(result.LeaseState),
        std::move(result.LeaseStatus),
        DeserializeMetadata(result.Properties)};
  }

  PathLeaseResponse PathClient::Lease(const PathLeaseOptions& options) const
  {
    DataLakeRestClient::Path::LeaseOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.XMsLeaseAction = options.LeaseAction;
    protocolLayerOptions.ProposedLeaseIdOptional = options.ProposedLeaseId;
    protocolLayerOptions.XMsLeaseDuration = options.LeaseDuration;
    protocolLayerOptions.XMsLeaseBreakPeriod = options.LeaseBreakPeriod;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Path::Lease(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  ReadPathResponse PathClient::Read(const PathReadOptions& options) const
  {
    DataLakeRestClient::Path::ReadOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.XMsRangeGetContentMd5 = options.RangeGetContentMd5;
    protocolLayerOptions.LeaseIdOptional = options.LeaseId;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::Path::Read(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    return ReadPathResponse{
        result.BodyStream,
        std::move(result.AcceptRanges),
        std::move(result.CacheControl),
        std::move(result.ContentDisposition),
        std::move(result.ContentEncoding),
        std::move(result.ContentLanguage),
        std::move(result.ContentLength),
        std::move(result.ContentRange),
        std::move(result.ContentType),
        std::move(result.ContentMD5),
        std::move(result.Date),
        std::move(result.ETag),
        std::move(result.LastModified),
        std::move(result.RequestId),
        std::move(result.Version),
        std::move(result.ResourceType),
        std::move(result.LeaseDuration),
        std::move(result.LeaseState),
        std::move(result.LeaseStatus),
        std::move(result.XMsContentMd5),
        DeserializeMetadata(result.Properties)};
  }
}}} // namespace Azure::Storage::DataLake

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

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  Acl Acl::FromString(const std::string& aclString)
  {
    std::string::const_iterator cur = aclString.begin();
    std::string scope = Details::GetSubstringTillDelimiter(':', aclString, cur);
    std::string type = Details::GetSubstringTillDelimiter(':', aclString, cur);
    std::string id = Details::GetSubstringTillDelimiter(':', aclString, cur);
    std::string permissions = Details::GetSubstringTillDelimiter(':', aclString, cur);

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
      result.emplace_back(
          FromString(Details::GetSubstringTillDelimiter(',', dataLakeAclsString, cur)));
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
  {
    Details::InitializeUrisFromServiceUri(pathUri, m_dfsUri, m_blobUri);

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
  {
    Details::InitializeUrisFromServiceUri(pathUri, m_dfsUri, m_blobUri);

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
  {
    Details::InitializeUrisFromServiceUri(pathUri, m_dfsUri, m_blobUri);

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
    return DataLakeRestClient::Path::SetAccessControl(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  SetPathHttpHeadersResponse PathClient::SetHttpHeaders(
      DataLakeHttpHeaders httpHeaders,
      const SetPathHttpHeadersOptions& options) const
  {
    DataLakeRestClient::Path::UpdateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Action = PathUpdateAction::SetProperties;
    protocolLayerOptions.CacheControl = httpHeaders.CacheControl;
    protocolLayerOptions.ContentType = httpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = httpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = httpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = httpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
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

  PathInfo PathClient::Create(PathResourceType type, const PathCreateOptions& options) const
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
    auto result = DataLakeRestClient::Path::Create(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    auto ret = PathInfo();
    ret.Date = std::move(result.Date);
    ret.ETag = std::move(result.ETag);
    ret.LastModified = std::move(result.LastModified);
    ret.RequestId = std::move(result.RequestId);
    ret.Version = std::move(result.Version);
    ret.ContentLength = std::move(result.ContentLength);
    return ret;
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
}}}} // namespace Azure::Storage::Files::DataLake

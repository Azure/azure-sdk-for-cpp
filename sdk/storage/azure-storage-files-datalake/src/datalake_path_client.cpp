//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_path_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "private/datalake_constants.hpp"
#include "private/datalake_utilities.hpp"
#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakePathClient DataLakePathClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& path,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto pathUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    pathUrl.AppendPath(_internal::UrlEncodePath(fileSystemName));
    pathUrl.AppendPath(_internal::UrlEncodePath(path));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakePathClient(
          pathUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakePathClient(pathUrl.GetAbsoluteUrl(), options);
    }
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_pathUrl(pathUrl), m_blobClient(
                                _detail::GetBlobUrlFromUrl(pathUrl),
                                credential,
                                _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
  {
    DataLakeClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_pathUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_pathUrl(pathUrl), m_blobClient(
                                _detail::GetBlobUrlFromUrl(pathUrl),
                                credential,
                                _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_pathUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(_internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUrl,
      const DataLakeClientOptions& options)
      : m_pathUrl(pathUrl),
        m_blobClient(_detail::GetBlobUrlFromUrl(pathUrl), _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_pathUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  Azure::Response<Models::SetPathAccessControlListResult> DataLakePathClient::SetAccessControlList(
      std::vector<Models::Acl> acls,
      const SetPathAccessControlListOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::SetPathAccessControlListOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::PathClient::SetAccessControlList(
        *m_pipeline, m_pathUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetPathPermissionsResult> DataLakePathClient::SetPermissions(
      std::string permissions,
      const SetPathPermissionsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::SetPathAccessControlListOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Permissions = permissions;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::PathClient::SetAccessControlList(
        *m_pipeline, m_pathUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetPathHttpHeadersResult> DataLakePathClient::SetHttpHeaders(
      Models::PathHttpHeaders httpHeaders,
      const SetPathHttpHeadersOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobHttpHeadersOptions blobOptions;
    Blobs::Models::BlobHttpHeaders blobHttpHeaders;
    blobHttpHeaders.CacheControl = httpHeaders.CacheControl;
    blobHttpHeaders.ContentType = httpHeaders.ContentType;
    blobHttpHeaders.ContentDisposition = httpHeaders.ContentDisposition;
    blobHttpHeaders.ContentEncoding = httpHeaders.ContentEncoding;
    blobHttpHeaders.ContentLanguage = httpHeaders.ContentLanguage;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto response = m_blobClient.SetHttpHeaders(blobHttpHeaders, blobOptions, context);
    Models::SetPathHttpHeadersResult ret;
    ret.ETag = std::move(response.Value.ETag);
    ret.LastModified = std::move(response.Value.LastModified);
    return Azure::Response<Models::SetPathHttpHeadersResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::CreatePathResult> DataLakePathClient::Create(
      Models::PathResourceType type,
      const CreatePathOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::CreatePathOptions protocolLayerOptions;
    protocolLayerOptions.Resource = type;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.CacheControl = options.HttpHeaders.CacheControl;
    protocolLayerOptions.ContentType = options.HttpHeaders.ContentType;
    protocolLayerOptions.ContentDisposition = options.HttpHeaders.ContentDisposition;
    protocolLayerOptions.ContentEncoding = options.HttpHeaders.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.HttpHeaders.ContentLanguage;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Properties = _detail::SerializeMetadata(options.Metadata);
    protocolLayerOptions.Umask = options.Umask;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.ProposedLeaseId = options.LeaseId;
    if (options.Acls.HasValue())
    {
      protocolLayerOptions.Acl = Models::Acl::SerializeAcls(options.Acls.Value());
    }
    if (options.LeaseDuration.HasValue())
    {
      protocolLayerOptions.LeaseDuration = static_cast<int64_t>(options.LeaseDuration->count());
    }
    AZURE_ASSERT_MSG(
        !(options.ScheduleDeletionOptions.ExpiresOn.HasValue()
          && options.ScheduleDeletionOptions.TimeToExpire.HasValue()),
        "ExpiresOn and TimeToExpire are mutually exclusive.");
    if (options.ScheduleDeletionOptions.ExpiresOn.HasValue())
    {
      protocolLayerOptions.ExpiryOptions
          = Files::DataLake::ScheduleFileExpiryOriginType::Absolute.ToString();
      protocolLayerOptions.ExpiresOn = options.ScheduleDeletionOptions.ExpiresOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc1123);
    }
    else if (options.ScheduleDeletionOptions.TimeToExpire.HasValue())
    {
      protocolLayerOptions.ExpiryOptions
          = Files::DataLake::ScheduleFileExpiryOriginType::RelativeToNow.ToString();
      protocolLayerOptions.ExpiresOn
          = std::to_string(options.ScheduleDeletionOptions.TimeToExpire.Value().count());
    }
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm.ToString();
    }
    return _detail::PathClient::Create(*m_pipeline, m_pathUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CreatePathResult> DataLakePathClient::CreateIfNotExists(
      Models::PathResourceType type,
      const CreatePathOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      auto createOptions = options;
      createOptions.AccessConditions.IfNoneMatch = Azure::ETag::Any();
      return Create(type, createOptions, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::DataLakePathAlreadyExists)
      {
        Models::CreatePathResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreatePathResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeletePathResult> DataLakePathClient::Delete(
      const DeletePathOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::DeletePathOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.Recursive = options.Recursive;
    return _detail::PathClient::Delete(*m_pipeline, m_pathUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::DeletePathResult> DataLakePathClient::DeleteIfExists(
      const DeletePathOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::DataLakeFilesystemNotFound
          || e.ErrorCode == _detail::DataLakePathNotFound)
      {
        Models::DeletePathResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeletePathResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::PathProperties> DataLakePathClient::GetProperties(
      const GetPathPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobPropertiesOptions blobOptions;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto response = m_blobClient.GetProperties(blobOptions, context);
    Models::PathProperties ret;
    ret.ETag = std::move(response.Value.ETag);
    ret.LastModified = std::move(response.Value.LastModified);
    ret.CreatedOn = std::move(response.Value.CreatedOn);
    ret.Metadata = std::move(response.Value.Metadata);
    ret.LeaseDuration = std::move(response.Value.LeaseDuration);
    ret.LeaseState = std::move(response.Value.LeaseState);
    ret.LeaseStatus = std::move(response.Value.LeaseStatus);
    ret.HttpHeaders = std::move(response.Value.HttpHeaders);
    ret.IsServerEncrypted = response.Value.IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(response.Value.EncryptionKeySha256);
    ret.EncryptionScope = std::move(response.Value.EncryptionScope);
    ret.CopyId = std::move(response.Value.CopyId);
    ret.CopySource = std::move(response.Value.CopySource);
    ret.CopyStatus = std::move(response.Value.CopyStatus);
    ret.CopyProgress = std::move(response.Value.CopyProgress);
    ret.CopyCompletedOn = std::move(response.Value.CopyCompletedOn);
    ret.ExpiresOn = std::move(response.Value.ExpiresOn);
    ret.LastAccessedOn = std::move(response.Value.LastAccessedOn);
    ret.FileSize = response.Value.BlobSize;
    ret.ArchiveStatus = std::move(response.Value.ArchiveStatus);
    ret.RehydratePriority = std::move(response.Value.RehydratePriority);
    ret.CopyStatusDescription = std::move(response.Value.CopyStatusDescription);
    ret.IsIncrementalCopy = std::move(response.Value.IsIncrementalCopy);
    ret.IncrementalCopyDestinationSnapshot
        = std::move(response.Value.IncrementalCopyDestinationSnapshot);
    ret.VersionId = std::move(response.Value.VersionId);
    ret.IsCurrentVersion = std::move(response.Value.IsCurrentVersion);
    ret.IsDirectory = _detail::MetadataIncidatesIsDirectory(ret.Metadata);
    return Azure::Response<Models::PathProperties>(std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::PathAccessControlList> DataLakePathClient::GetAccessControlList(
      const GetPathAccessControlListOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::GetPathAccessControlListOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto response = _detail::PathClient::GetAccessControlList(
        *m_pipeline, m_pathUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
    Azure::Nullable<std::vector<Models::Acl>> acl;
    if (!response.Value.Acl.empty())
    {
      acl = Models::Acl::DeserializeAcls(response.Value.Acl);
    }
    Models::PathAccessControlList ret;
    if (!acl.HasValue())
    {
      throw Azure::Core::RequestFailedException(
          "Got null value returned when getting access control.");
    }
    ret.Acls = std::move(acl.Value());
    ret.Owner = std::move(response.Value.Owner);
    ret.Group = std::move(response.Value.Group);
    ret.Permissions = std::move(response.Value.Permissions);
    return Azure::Response<Models::PathAccessControlList>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::SetPathMetadataResult> DataLakePathClient::SetMetadata(
      Storage::Metadata metadata,
      const SetPathMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobMetadataOptions blobOptions;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto response = m_blobClient.SetMetadata(std::move(metadata), blobOptions, context);
    Models::SetPathMetadataResult ret;
    ret.ETag = std::move(response.Value.ETag);
    ret.LastModified = std::move(response.Value.LastModified);
    return Azure::Response<Models::SetPathMetadataResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  SetPathAccessControlListRecursivePagedResponse
  DataLakePathClient::SetAccessControlListRecursiveInternal(
      Models::_detail::PathSetAccessControlListRecursiveMode mode,
      const std::vector<Models::Acl>& acls,
      const SetPathAccessControlListRecursiveOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::PathClient::SetPathAccessControlListRecursiveOptions protocolLayerOptions;
    protocolLayerOptions.Mode = mode.ToString();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxRecords = options.PageSizeHint;
    protocolLayerOptions.ForceFlag = options.ContinueOnFailure;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    auto response = _detail::PathClient::SetAccessControlListRecursive(
        *m_pipeline, m_pathUrl, protocolLayerOptions, context);

    SetPathAccessControlListRecursivePagedResponse pagedResponse;
    pagedResponse.NumberOfSuccessfulFiles = response.Value.NumberOfSuccessfulDirectories;
    pagedResponse.NumberOfSuccessfulFiles = response.Value.NumberOfSuccessfulFiles;
    pagedResponse.NumberOfFailures = response.Value.NumberOfFailures;
    pagedResponse.FailedEntries = std::move(response.Value.FailedEntries);
    pagedResponse.m_dataLakePathClient = std::make_shared<DataLakePathClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_acls = acls;
    pagedResponse.m_mode = mode;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

}}}} // namespace Azure::Storage::Files::DataLake

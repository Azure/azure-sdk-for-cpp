// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_path_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_constants.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace {
    Models::LeaseStateType FromBlobLeaseState(Blobs::Models::BlobLeaseState state)
    {
      if (state == Blobs::Models::BlobLeaseState::Available)
      {
        return Models::LeaseStateType::Available;
      }
      if (state == Blobs::Models::BlobLeaseState::Breaking)
      {
        return Models::LeaseStateType::Breaking;
      }
      if (state == Blobs::Models::BlobLeaseState::Broken)
      {
        return Models::LeaseStateType::Broken;
      }
      if (state == Blobs::Models::BlobLeaseState::Expired)
      {
        return Models::LeaseStateType::Expired;
      }
      if (state == Blobs::Models::BlobLeaseState::Leased)
      {
        return Models::LeaseStateType::Leased;
      }
      return Models::LeaseStateType();
    }

    Models::LeaseStatusType FromBlobLeaseStatus(Blobs::Models::BlobLeaseStatus status)
    {
      if (status == Blobs::Models::BlobLeaseStatus::Locked)
      {
        return Models::LeaseStatusType::Locked;
      }
      if (status == Blobs::Models::BlobLeaseStatus::Unlocked)
      {
        return Models::LeaseStatusType::Unlocked;
      }
      return Models::LeaseStatusType();
    }
  } // namespace

  DataLakePathClient DataLakePathClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& path,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto pathUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    pathUrl.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    pathUrl.AppendPath(Storage::Details::UrlEncodePath(path));

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
                                Details::GetBlobUrlFromUrl(pathUrl),
                                credential,
                                Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Storage::Details::SharedKeyPolicy>(credential),
            options));
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_pathUrl(pathUrl), m_blobClient(
                                Details::GetBlobUrlFromUrl(pathUrl),
                                credential,
                                Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    Azure::Core::Http::TokenRequestOptions tokenOptions;
    tokenOptions.Scopes.emplace_back(Storage::Details::StorageScope);
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
                credential, tokenOptions),
            options));
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUrl,
      const DataLakeClientOptions& options)
      : m_pathUrl(pathUrl),
        m_blobClient(Details::GetBlobUrlFromUrl(pathUrl), Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            nullptr,
            options));
  }

  Azure::Core::Response<Models::SetDataLakePathAccessControlListResult>
  DataLakePathClient::SetAccessControlList(
      std::vector<Models::Acl> acls,
      const SetDataLakePathAccessControlListOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::DataLakeRestClient::Path::SetAccessControl(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDataLakePathPermissionsResult>
  DataLakePathClient::SetPermissions(
      std::string permissions,
      const SetDataLakePathPermissionsOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Permissions = permissions;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::DataLakeRestClient::Path::SetAccessControl(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDataLakePathHttpHeadersResult>
  DataLakePathClient::SetHttpHeaders(
      Models::PathHttpHeaders httpHeaders,
      const SetDataLakePathHttpHeadersOptions& options,
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
    auto result = m_blobClient.SetHttpHeaders(blobHttpHeaders, blobOptions, context);
    Models::SetDataLakePathHttpHeadersResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakePathHttpHeadersResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakePathResult> DataLakePathClient::Create(
      Models::PathResourceType type,
      const CreateDataLakePathOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
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
    auto result = Details::DataLakeRestClient::Path::Create(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateDataLakePathResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified.GetValue());
    ret.FileSize = std::move(result->ContentLength);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::CreateDataLakePathResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakePathResult> DataLakePathClient::CreateIfNotExists(
      Models::PathResourceType type,
      const CreateDataLakePathOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      auto createOptions = options;
      createOptions.AccessConditions.IfNoneMatch = Azure::Core::ETag::Any();
      return Create(type, createOptions, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::DataLakePathAlreadyExists)
      {
        Models::CreateDataLakePathResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateDataLakePathResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteDataLakePathResult> DataLakePathClient::Delete(
      const DeleteDataLakePathOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RecursiveOptional = options.Recursive;
    auto result = Details::DataLakeRestClient::Path::Delete(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteDataLakePathResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakePathResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakePathResult> DataLakePathClient::DeleteIfExists(
      const DeleteDataLakePathOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::DataLakeFilesystemNotFound
          || e.ErrorCode == Details::DataLakePathNotFound)
      {
        Models::DeleteDataLakePathResult ret;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteDataLakePathResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetDataLakePathPropertiesResult> DataLakePathClient::GetProperties(
      const GetDataLakePathPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobPropertiesOptions blobOptions;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.GetProperties(blobOptions, context);
    Models::GetDataLakePathPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.CreatedOn = std::move(result->CreatedOn);
    ret.Metadata = std::move(result->Metadata);
    if (result->LeaseDuration.HasValue())
    {
      ret.LeaseDuration = Models::LeaseDurationType(result->LeaseDuration.GetValue().Get());
    }
    ret.LeaseState = result->LeaseState.HasValue()
        ? FromBlobLeaseState(result->LeaseState.GetValue())
        : ret.LeaseState;
    ret.LeaseStatus = result->LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->LeaseStatus.GetValue())
        : ret.LeaseStatus;
    ret.HttpHeaders.CacheControl = std::move(result->HttpHeaders.CacheControl);
    ret.HttpHeaders.ContentDisposition = std::move(result->HttpHeaders.ContentDisposition);
    ret.HttpHeaders.ContentEncoding = std::move(result->HttpHeaders.ContentEncoding);
    ret.HttpHeaders.ContentLanguage = std::move(result->HttpHeaders.ContentLanguage);
    ret.HttpHeaders.ContentType = std::move(result->HttpHeaders.ContentType);
    ret.IsServerEncrypted = result->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    ret.IsAccessTierInferred = std::move(result->IsAccessTierInferred);
    ret.AccessTierChangedOn = std::move(result->AccessTierChangedOn);
    ret.CopyId = std::move(result->CopyId);
    ret.CopySource = std::move(result->CopySource);
    ret.CopyStatus = std::move(result->CopyStatus);
    ret.CopyProgress = std::move(result->CopyProgress);
    ret.CopyCompletedOn = std::move(result->CopyCompletedOn);
    ret.ExpiresOn = std::move(result->ExpiresOn);
    ret.LastAccessedOn = std::move(result->LastAccessedOn);
    ret.FileSize = result->BlobSize;
    ret.RequestId = std::move(result->RequestId);
    ret.ArchiveStatus = std::move(result->ArchiveStatus);
    ret.RehydratePriority = std::move(result->RehydratePriority);
    ret.CopyStatusDescription = std::move(result->CopyStatusDescription);
    ret.IsIncrementalCopy = std::move(result->IsIncrementalCopy);
    ret.IncrementalCopyDestinationSnapshot = std::move(result->IncrementalCopyDestinationSnapshot);
    ret.VersionId = std::move(result->VersionId);
    ret.IsCurrentVersion = std::move(result->IsCurrentVersion);
    ret.IsDirectory = Details::MetadataIncidatesIsDirectory(ret.Metadata);
    return Azure::Core::Response<Models::GetDataLakePathPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::GetDataLakePathAccessControlListResult>
  DataLakePathClient::GetAccessControlList(
      const GetDataLakePathAccessControlListOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::GetPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.Action = Models::PathGetPropertiesAction::GetAccessControl;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = Details::DataLakeRestClient::Path::GetProperties(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
    Azure::Core::Nullable<std::vector<Models::Acl>> acl;
    if (result->Acl.HasValue())
    {
      acl = Models::Acl::DeserializeAcls(result->Acl.GetValue());
    }
    Models::GetDataLakePathAccessControlListResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    if (!acl.HasValue())
    {
      throw Azure::Core::RequestFailedException(
          "Got null value returned when getting access control.");
    }
    ret.Acls = std::move(acl.GetValue());
    if (result->Owner.HasValue())
    {
      ret.Owner = result->Owner.GetValue();
    }
    if (result->Group.HasValue())
    {
      ret.Group = result->Group.GetValue();
    }
    if (result->Permissions.HasValue())
    {
      ret.Permissions = result->Permissions.GetValue();
    }
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::GetDataLakePathAccessControlListResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakePathMetadataResult> DataLakePathClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDataLakePathMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobMetadataOptions blobOptions;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.SetMetadata(std::move(metadata), blobOptions, context);
    Models::SetDataLakePathMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakePathMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakePathAccessControlListRecursiveSinglePageResult>
  DataLakePathClient::SetAccessControlListRecursiveSinglePageInternal(
      Models::PathSetAccessControlRecursiveMode mode,
      const std::vector<Models::Acl>& acls,
      const SetDataLakePathAccessControlListRecursiveSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlRecursiveOptions protocolLayerOptions;
    protocolLayerOptions.Mode = mode;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxRecords = options.PageSizeHint;
    protocolLayerOptions.ForceFlag = options.ContinueOnFailure;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    return Details::DataLakeRestClient::Path::SetAccessControlRecursive(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_path_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

#include "azure/storage/files/datalake/datalake_constants.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobClientOptions GetBlobClientOptions(const DataLakeClientOptions& options)
    {
      Blobs::BlobClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(p->Clone());
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(p->Clone());
      }
      blobOptions.RetryOptions = options.RetryOptions;
      blobOptions.RetryOptions.SecondaryHostForRetryReads
          = Details::GetBlobUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
      return blobOptions;
    }

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
    auto pathUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    pathUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    pathUri.AppendPath(Storage::Details::UrlEncodePath(path));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakePathClient(
          pathUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakePathClient(pathUri.GetAbsoluteUrl(), options);
    }
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(pathUri)),
        m_blobClient(Details::GetBlobUriFromUri(pathUri), credential, GetBlobClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(pathUri)),
        m_blobClient(Details::GetBlobUriFromUri(pathUri), credential, GetBlobClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::Http::BearerTokenAuthenticationPolicy>(
        credential, Azure::Storage::Details::StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakePathClient::DataLakePathClient(
      const std::string& pathUri,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(pathUri)),
        m_blobClient(Details::GetBlobUriFromUri(pathUri), GetBlobClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    Azure::Core::Http::TransportPolicyOptions transportPolicyOptions;
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<Models::SetDataLakePathAccessControlResult>
  DataLakePathClient::SetAccessControl(
      std::vector<Models::Acl> acls,
      const SetDataLakePathAccessControlOptions& options) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.Owner = options.Owner;
    protocolLayerOptions.Group = options.Group;
    protocolLayerOptions.Permissions = options.Permissions;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::DataLakeRestClient::Path::SetAccessControl(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDataLakePathHttpHeadersResult>
  DataLakePathClient::SetHttpHeaders(
      Models::PathHttpHeaders httpHeaders,
      const SetDataLakePathHttpHeadersOptions& options) const
  {
    Blobs::SetBlobHttpHeadersOptions blobOptions;
    Blobs::Models::BlobHttpHeaders blobHttpHeaders;
    blobOptions.Context = options.Context;
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
    auto result = m_blobClient.SetHttpHeaders(blobHttpHeaders, blobOptions);
    Models::SetDataLakePathHttpHeadersResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<Models::SetDataLakePathHttpHeadersResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakePathResult> DataLakePathClient::Create(
      Models::PathResourceType type,
      const CreateDataLakePathOptions& options) const
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
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::CreateDataLakePathResult ret;
    ret.ETag = std::move(result->ETag.GetValue());
    ret.LastModified = std::move(result->LastModified.GetValue());
    ret.ContentLength = std::move(result->ContentLength);
    return Azure::Core::Response<Models::CreateDataLakePathResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakePathResult> DataLakePathClient::CreateIfNotExists(
      Models::PathResourceType type,
      const CreateDataLakePathOptions& options) const
  {
    try
    {
      auto createOptions = options;
      createOptions.AccessConditions.IfNoneMatch = ETagWildcard;
      return Create(type, createOptions);
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
      const DeleteDataLakePathOptions& options) const
  {
    Details::DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RecursiveOptional = options.Recursive;
    auto result = Details::DataLakeRestClient::Path::Delete(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::DeleteDataLakePathResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Deleted = true;
    return Azure::Core::Response<Models::DeleteDataLakePathResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakePathResult> DataLakePathClient::DeleteIfExists(
      const DeleteDataLakePathOptions& options) const
  {
    try
    {
      return Delete(options);
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
      const GetDataLakePathPropertiesOptions& options) const
  {
    Blobs::GetBlobPropertiesOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.GetProperties(blobOptions);
    Models::GetDataLakePathPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.CreatedOn = std::move(result->CreatedOn);
    ret.Metadata = std::move(result->Metadata);
    ret.LeaseDuration = std::move(result->LeaseDuration);
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
    ret.ServerEncrypted = result->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    ret.AccessTierInferred = std::move(result->IsAccessTierInferred);
    ret.AccessTierChangedOn = std::move(result->AccessTierChangedOn);
    ret.CopyId = std::move(result->CopyId);
    ret.CopySource = std::move(result->CopySource);
    ret.CopyStatus = std::move(result->CopyStatus);
    ret.CopyProgress = std::move(result->CopyProgress);
    ret.CopyCompletedOn = std::move(result->CopyCompletedOn);
    ret.ExpiresOn = std::move(result->ExpiriesOn);
    ret.LastAccessedOn = std::move(result->LastAccessedOn);
    ret.ContentLength = result->ContentLength;
    return Azure::Core::Response<Models::GetDataLakePathPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::GetDataLakePathAccessControlResult>
  DataLakePathClient::GetAccessControls(const GetDataLakePathAccessControlOptions& options) const
  {
    Details::DataLakeRestClient::Path::GetPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.Action = Models::PathGetPropertiesAction::GetAccessControl;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = Details::DataLakeRestClient::Path::GetProperties(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    Azure::Core::Nullable<std::vector<Models::Acl>> acl;
    if (result->Acl.HasValue())
    {
      acl = Models::Acl::DeserializeAcls(result->Acl.GetValue());
    }
    Models::GetDataLakePathAccessControlResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    if (!acl.HasValue())
    {
      throw std::runtime_error("Got null value returned when getting access control.");
    }
    ret.Acls = std::move(acl.GetValue());
    return Azure::Core::Response<Models::GetDataLakePathAccessControlResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakePathMetadataResult> DataLakePathClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDataLakePathMetadataOptions& options) const
  {
    Blobs::SetBlobMetadataOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.SetMetadata(std::move(metadata), blobOptions);
    Models::SetDataLakePathMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<Models::SetDataLakePathMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }
}}}} // namespace Azure::Storage::Files::DataLake

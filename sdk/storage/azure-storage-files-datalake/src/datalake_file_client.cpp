// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_client.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

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
    Models::PathHttpHeaders FromBlobHttpHeaders(Blobs::Models::BlobHttpHeaders headers)
    {
      Models::PathHttpHeaders ret;
      ret.CacheControl = std::move(headers.CacheControl);
      ret.ContentDisposition = std::move(headers.ContentDisposition);
      ret.ContentEncoding = std::move(headers.ContentEncoding);
      ret.ContentLanguage = std::move(headers.ContentLanguage);
      ret.ContentType = std::move(headers.ContentType);
      ret.ContentHash = std::move(headers.ContentHash);
      return ret;
    }

    Blobs::Models::BlobHttpHeaders FromPathHttpHeaders(Models::PathHttpHeaders headers)
    {
      Blobs::Models::BlobHttpHeaders ret;
      ret.CacheControl = std::move(headers.CacheControl);
      ret.ContentDisposition = std::move(headers.ContentDisposition);
      ret.ContentEncoding = std::move(headers.ContentEncoding);
      ret.ContentLanguage = std::move(headers.ContentLanguage);
      ret.ContentType = std::move(headers.ContentType);
      return ret;
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

  DataLakeFileClient DataLakeFileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& fileName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    fileUri.AppendPath(Storage::Details::UrlEncodePath(fileName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeFileClient(
          fileUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeFileClient(fileUri.GetAbsoluteUrl(), options);
    }
  }

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.AsBlockBlobClient())
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
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

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.AsBlockBlobClient())
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
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

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUri,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUri, options), m_blockBlobClient(m_blobClient.AsBlockBlobClient())
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<Models::AppendDataLakeFileResult> DataLakeFileClient::Append(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const AppendDataLakeFileOptions& options) const
  {
    Details::DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = content->Length();
    if (options.TransactionalContentHash.HasValue())
    {
      if (options.TransactionalContentHash.GetValue().Algorithm == HashAlgorithm::Crc64)
      {
        protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentHash;
      }
      else
      {
        protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentHash;
      }
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return Details::DataLakeRestClient::Path::AppendData(
        m_dfsUrl, *content, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::FlushDataLakeFileResult> DataLakeFileClient::Flush(
      int64_t position,
      const FlushDataLakeFileOptions& options) const
  {
    Details::DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = position;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    if (options.ContentHash.HasValue()
        && options.ContentHash.GetValue().Algorithm != HashAlgorithm::Md5)
    {
      std::abort();
    }
    protocolLayerOptions.ContentMd5 = options.ContentHash;
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
    return Details::DataLakeRestClient::Path::FlushData(
        m_dfsUrl, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteDataLakeFileResult> DataLakeFileClient::Delete(
      const DeleteDataLakeFileOptions& options) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    auto result = DataLakePathClient::Delete(deleteOptions);
    Models::DeleteDataLakeFileResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeFileResult> DataLakeFileClient::DeleteIfExists(
      const DeleteDataLakeFileOptions& options) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    auto result = DataLakePathClient::DeleteIfExists(deleteOptions);
    Models::DeleteDataLakeFileResult ret;
    ret.Deleted = result->Deleted;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DownloadDataLakeFileResult> DataLakeFileClient::Download(
      const DownloadDataLakeFileOptions& options) const
  {
    Blobs::DownloadBlobOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Range = options.Range;
    blobOptions.RangeHashAlgorithm = options.RangeHashAlgorithm;
    blobOptions.Range = options.Range;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.Download(blobOptions);
    Models::DownloadDataLakeFileResult ret;
    ret.Body = std::move(result->BodyStream);
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.ContentRange = std::move(result->ContentRange);
    ret.FileSize = result->BlobSize;
    ret.TransactionalContentHash = std::move(result->TransactionalContentHash);
    ret.ETag = std::move(result->Details.ETag);
    ret.LastModified = std::move(result->Details.LastModified);
    if (result->Details.LeaseDuration.HasValue())
    {
      ret.LeaseDuration = Models::LeaseDurationType(result->Details.LeaseDuration.GetValue().Get());
    }
    ret.LeaseState = result->Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result->Details.LeaseState.GetValue())
        : ret.LeaseState;
    ret.LeaseStatus = result->Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->Details.LeaseStatus.GetValue())
        : ret.LeaseStatus;
    ret.Metadata = std::move(result->Details.Metadata);
    ret.CreatedOn = std::move(result->Details.CreatedOn);
    ret.ExpiresOn = std::move(result->Details.ExpiresOn);
    ret.LastAccessedOn = std::move(result->Details.LastAccessedOn);
    ret.CopyId = std::move(result->Details.CopyId);
    ret.CopySource = std::move(result->Details.CopySource);
    ret.CopyStatus = std::move(result->Details.CopyStatus);
    ret.CopyStatusDescription = std::move(result->Details.CopyStatusDescription);
    ret.CopyProgress = std::move(result->Details.CopyProgress);
    ret.CopyCompletedOn = std::move(result->Details.CopyCompletedOn);
    ret.VersionId = std::move(result->Details.VersionId);
    ret.IsCurrentVersion = std::move(result->Details.IsCurrentVersion);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DownloadDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::UploadDataLakeFileFromResult> DataLakeFileClient::UploadFrom(
      const std::string& fileName,
      const UploadDataLakeFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.TransferOptions.SingleUploadThreshold
        = options.TransferOptions.SingleUploadThreshold;
    blobOptions.TransferOptions.ChunkSize = options.TransferOptions.ChunkSize;
    blobOptions.TransferOptions.Concurrency = options.TransferOptions.Concurrency;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    return m_blockBlobClient.UploadFrom(fileName, blobOptions);
  }

  Azure::Core::Response<Models::UploadDataLakeFileFromResult> DataLakeFileClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadDataLakeFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.TransferOptions.SingleUploadThreshold
        = options.TransferOptions.SingleUploadThreshold;
    blobOptions.TransferOptions.ChunkSize = options.TransferOptions.ChunkSize;
    blobOptions.TransferOptions.Concurrency = options.TransferOptions.Concurrency;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    return m_blockBlobClient.UploadFrom(buffer, bufferSize, blobOptions);
  }

  Azure::Core::Response<Models::DownloadDataLakeFileToResult> DataLakeFileClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadDataLakeFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(buffer, bufferSize, options);
    Models::DownloadDataLakeFileToResult ret;
    ret.ETag = std::move(result->Details.ETag);
    ret.LastModified = std::move(result->Details.LastModified);
    ret.ContentLength = result->ContentRange.Length.GetValue();
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.Metadata = std::move(result->Details.Metadata);
    ret.ServerEncrypted = result->Details.IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->Details.EncryptionKeySha256);
    return Azure::Core::Response<Models::DownloadDataLakeFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DownloadDataLakeFileToResult> DataLakeFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadDataLakeFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(fileName, options);
    Models::DownloadDataLakeFileToResult ret;
    ret.ETag = std::move(result->Details.ETag);
    ret.LastModified = std::move(result->Details.LastModified);
    ret.ContentLength = result->ContentRange.Length.GetValue();
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.Metadata = std::move(result->Details.Metadata);
    ret.ServerEncrypted = result->Details.IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->Details.EncryptionKeySha256);
    return Azure::Core::Response<Models::DownloadDataLakeFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ScheduleDataLakeFileDeletionResult>
  DataLakeFileClient::ScheduleDeletion(
      ScheduleDataLakeFileExpiryOriginType expiryOrigin,
      const ScheduleDataLakeFileDeletionOptions& options) const
  {
    Blobs::Details::BlobRestClient::Blob::SetBlobExpiryOptions protocolLayerOptions;
    protocolLayerOptions.ExpiryOrigin = expiryOrigin;
    if (options.ExpiresOn.HasValue() && options.TimeToExpireInMs.HasValue())
    {
      // ExpiresOn and TimeToExpireInMs should be mutually exlusive.
      std::abort();
    }
    if (options.ExpiresOn.HasValue())
    {
      protocolLayerOptions.ExpiryTime = options.ExpiresOn;
    }
    else if (options.TimeToExpireInMs.HasValue())
    {
      protocolLayerOptions.ExpiryTime = std::to_string(options.TimeToExpireInMs.GetValue());
    }
    return Blobs::Details::BlobRestClient::Blob::ScheduleDeletion(
        options.Context, *m_pipeline, m_blobClient.m_blobUrl, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

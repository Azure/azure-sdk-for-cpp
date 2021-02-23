// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_client.hpp"

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
    auto fileUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileUrl.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    fileUrl.AppendPath(Storage::Details::UrlEncodePath(fileName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeFileClient(
          fileUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeFileClient(fileUrl.GetAbsoluteUrl(), options);
    }
  }

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUrl, credential, options)
  {
  }

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUrl, credential, options)
  {
  }

  DataLakeFileClient::DataLakeFileClient(
      const std::string& fileUrl,
      const DataLakeClientOptions& options)
      : DataLakePathClient(fileUrl, options)
  {
  }

  Azure::Core::Response<Models::AppendDataLakeFileResult> DataLakeFileClient::Append(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const AppendDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
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
        m_pathUrl, *content, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::FlushDataLakeFileResult> DataLakeFileClient::Flush(
      int64_t position,
      const FlushDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
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
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteDataLakeFileResult> DataLakeFileClient::Delete(
      const DeleteDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto result = DataLakePathClient::Delete(deleteOptions, context);
    Models::DeleteDataLakeFileResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeFileResult> DataLakeFileClient::DeleteIfExists(
      const DeleteDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto result = DataLakePathClient::DeleteIfExists(deleteOptions, context);
    Models::DeleteDataLakeFileResult ret;
    ret.Deleted = result->Deleted;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DownloadDataLakeFileResult> DataLakeFileClient::Download(
      const DownloadDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::DownloadBlobOptions blobOptions;
    blobOptions.Range = options.Range;
    blobOptions.RangeHashAlgorithm = options.RangeHashAlgorithm;
    blobOptions.Range = options.Range;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.Download(blobOptions, context);
    Models::DownloadDataLakeFileResult ret;
    ret.Body = std::move(result->BodyStream);
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.ContentRange = std::move(result->ContentRange);
    ret.FileSize = result->BlobSize;
    ret.TransactionalContentHash = std::move(result->TransactionalContentHash);
    ret.Details.ETag = std::move(result->Details.ETag);
    ret.Details.LastModified = std::move(result->Details.LastModified);
    if (result->Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDurationType(result->Details.LeaseDuration.GetValue().Get());
    }
    ret.Details.LeaseState = result->Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result->Details.LeaseState.GetValue())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result->Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->Details.LeaseStatus.GetValue())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result->Details.Metadata);
    ret.Details.CreatedOn = std::move(result->Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result->Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result->Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result->Details.CopyId);
    ret.Details.CopySource = std::move(result->Details.CopySource);
    ret.Details.CopyStatus = std::move(result->Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result->Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result->Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result->Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result->Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result->Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result->Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result->Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result->Details.IsServerEncrypted;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DownloadDataLakeFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::UploadDataLakeFileFromResult> DataLakeFileClient::UploadFrom(
      const std::string& fileName,
      const UploadDataLakeFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.TransferOptions.SingleUploadThreshold
        = options.TransferOptions.SingleUploadThreshold;
    blobOptions.TransferOptions.ChunkSize = options.TransferOptions.ChunkSize;
    blobOptions.TransferOptions.Concurrency = options.TransferOptions.Concurrency;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    return m_blobClient.AsBlockBlobClient().UploadFrom(fileName, blobOptions, context);
  }

  Azure::Core::Response<Models::UploadDataLakeFileFromResult> DataLakeFileClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadDataLakeFileFromOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.TransferOptions.SingleUploadThreshold
        = options.TransferOptions.SingleUploadThreshold;
    blobOptions.TransferOptions.ChunkSize = options.TransferOptions.ChunkSize;
    blobOptions.TransferOptions.Concurrency = options.TransferOptions.Concurrency;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    return m_blobClient.AsBlockBlobClient().UploadFrom(buffer, bufferSize, blobOptions, context);
  }

  Azure::Core::Response<Models::DownloadDataLakeFileToResult> DataLakeFileClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadDataLakeFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto result = m_blobClient.AsBlockBlobClient().DownloadTo(buffer, bufferSize, options, context);
    Models::DownloadDataLakeFileToResult ret;
    ret.ContentRange = std::move(result->ContentRange);
    ret.FileSize = result->BlobSize;
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.Details.ETag = std::move(result->Details.ETag);
    ret.Details.LastModified = std::move(result->Details.LastModified);
    if (result->Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDurationType(result->Details.LeaseDuration.GetValue().Get());
    }
    ret.Details.LeaseState = result->Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result->Details.LeaseState.GetValue())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result->Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->Details.LeaseStatus.GetValue())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result->Details.Metadata);
    ret.Details.CreatedOn = std::move(result->Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result->Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result->Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result->Details.CopyId);
    ret.Details.CopySource = std::move(result->Details.CopySource);
    ret.Details.CopyStatus = std::move(result->Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result->Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result->Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result->Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result->Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result->Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result->Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result->Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result->Details.IsServerEncrypted;
    return Azure::Core::Response<Models::DownloadDataLakeFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DownloadDataLakeFileToResult> DataLakeFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadDataLakeFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto result = m_blobClient.AsBlockBlobClient().DownloadTo(fileName, options, context);
    Models::DownloadDataLakeFileToResult ret;
    ret.ContentRange = std::move(result->ContentRange);
    ret.FileSize = result->BlobSize;
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result->Details.HttpHeaders));
    ret.Details.ETag = std::move(result->Details.ETag);
    ret.Details.LastModified = std::move(result->Details.LastModified);
    if (result->Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDurationType(result->Details.LeaseDuration.GetValue().Get());
    }
    ret.Details.LeaseState = result->Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result->Details.LeaseState.GetValue())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result->Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->Details.LeaseStatus.GetValue())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result->Details.Metadata);
    ret.Details.CreatedOn = std::move(result->Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result->Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result->Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result->Details.CopyId);
    ret.Details.CopySource = std::move(result->Details.CopySource);
    ret.Details.CopyStatus = std::move(result->Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result->Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result->Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result->Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result->Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result->Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result->Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result->Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result->Details.IsServerEncrypted;
    return Azure::Core::Response<Models::DownloadDataLakeFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ScheduleDataLakeFileDeletionResult>
  DataLakeFileClient::ScheduleDeletion(
      ScheduleDataLakeFileExpiryOriginType expiryOrigin,
      const ScheduleDataLakeFileDeletionOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::Details::BlobRestClient::Blob::SetBlobExpiryOptions protocolLayerOptions;
    protocolLayerOptions.ExpiryOrigin = expiryOrigin;
    if (options.ExpiresOn.HasValue() && options.TimeToExpire.HasValue())
    {
      // ExpiresOn and TimeToExpire should be mutually exlusive.
      std::abort();
    }
    if (options.ExpiresOn.HasValue())
    {
      protocolLayerOptions.ExpiryTime
          = options.ExpiresOn.GetValue().ToString(Azure::Core::DateTime::DateFormat::Rfc1123);
    }
    else if (options.TimeToExpire.HasValue())
    {
      protocolLayerOptions.ExpiryTime = std::to_string(options.TimeToExpire.GetValue().count());
    }
    return Blobs::Details::BlobRestClient::Blob::ScheduleDeletion(
        context, *m_pipeline, m_blobClient.m_blobUrl, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

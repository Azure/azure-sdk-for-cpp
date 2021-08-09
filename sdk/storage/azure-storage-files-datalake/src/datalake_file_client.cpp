// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "private/datalake_constants.hpp"
#include "private/datalake_utilities.hpp"

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

    Models::LeaseState FromBlobLeaseState(Blobs::Models::LeaseState state)
    {
      if (state == Blobs::Models::LeaseState::Available)
      {
        return Models::LeaseState::Available;
      }
      if (state == Blobs::Models::LeaseState::Breaking)
      {
        return Models::LeaseState::Breaking;
      }
      if (state == Blobs::Models::LeaseState::Broken)
      {
        return Models::LeaseState::Broken;
      }
      if (state == Blobs::Models::LeaseState::Expired)
      {
        return Models::LeaseState::Expired;
      }
      if (state == Blobs::Models::LeaseState::Leased)
      {
        return Models::LeaseState::Leased;
      }
      return Models::LeaseState();
    }

    Models::LeaseStatus FromBlobLeaseStatus(Blobs::Models::LeaseStatus status)
    {
      if (status == Blobs::Models::LeaseStatus::Locked)
      {
        return Models::LeaseStatus::Locked;
      }
      if (status == Blobs::Models::LeaseStatus::Unlocked)
      {
        return Models::LeaseStatus::Unlocked;
      }
      return Models::LeaseStatus();
    }
  } // namespace

  DataLakeFileClient DataLakeFileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& fileName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto fileUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileUrl.AppendPath(_internal::UrlEncodePath(fileSystemName));
    fileUrl.AppendPath(_internal::UrlEncodePath(fileName));

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
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
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

  Azure::Response<Models::AppendFileResult> DataLakeFileClient::Append(
      Azure::Core::IO::BodyStream& content,
      int64_t offset,
      const AppendFileOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = content.Length();
    if (options.TransactionalContentHash.HasValue())
    {
      if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
      {
        protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentHash;
      }
      else
      {
        protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentHash;
      }
    }
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return _detail::DataLakeRestClient::Path::AppendData(
        m_pathUrl, content, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::FlushFileResult> DataLakeFileClient::Flush(
      int64_t position,
      const FlushFileOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = position;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    if (options.ContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.ContentHash.Value().Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
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
    return _detail::DataLakeRestClient::Path::FlushData(
        m_pathUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::DeleteFileResult> DataLakeFileClient::Delete(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto result = DataLakePathClient::Delete(deleteOptions, context);
    Models::DeleteFileResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteFileResult>(std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteFileResult> DataLakeFileClient::DeleteIfExists(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto result = DataLakePathClient::DeleteIfExists(deleteOptions, context);
    Models::DeleteFileResult ret;
    ret.Deleted = result.Value.Deleted;
    return Azure::Response<Models::DeleteFileResult>(std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DownloadFileResult> DataLakeFileClient::Download(
      const DownloadFileOptions& options,
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
    Models::DownloadFileResult ret;
    ret.Body = std::move(result.Value.BodyStream);
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result.Value.Details.HttpHeaders));
    ret.ContentRange = std::move(result.Value.ContentRange);
    ret.FileSize = result.Value.BlobSize;
    ret.TransactionalContentHash = std::move(result.Value.TransactionalContentHash);
    ret.Details.ETag = std::move(result.Value.Details.ETag);
    ret.Details.LastModified = std::move(result.Value.Details.LastModified);
    if (result.Value.Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDuration(result.Value.Details.LeaseDuration.Value().ToString());
    }
    ret.Details.LeaseState = result.Value.Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result.Value.Details.LeaseState.Value())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result.Value.Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result.Value.Details.LeaseStatus.Value())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(result.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result.Value.Details.CopyId);
    ret.Details.CopySource = std::move(result.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(result.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::UploadFileFromResult> DataLakeFileClient::UploadFrom(
      const std::string& fileName,
      const UploadFileFromOptions& options,
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

  Azure::Response<Models::UploadFileFromResult> DataLakeFileClient::UploadFrom(
      const uint8_t* buffer,
      size_t bufferSize,
      const UploadFileFromOptions& options,
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

  Azure::Response<Models::DownloadFileToResult> DataLakeFileClient::DownloadTo(
      uint8_t* buffer,
      size_t bufferSize,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto result = m_blobClient.AsBlockBlobClient().DownloadTo(buffer, bufferSize, options, context);
    Models::DownloadFileToResult ret;
    ret.ContentRange = std::move(result.Value.ContentRange);
    ret.FileSize = result.Value.BlobSize;
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result.Value.Details.HttpHeaders));
    ret.Details.ETag = std::move(result.Value.Details.ETag);
    ret.Details.LastModified = std::move(result.Value.Details.LastModified);
    if (result.Value.Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDuration(result.Value.Details.LeaseDuration.Value().ToString());
    }
    ret.Details.LeaseState = result.Value.Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result.Value.Details.LeaseState.Value())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result.Value.Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result.Value.Details.LeaseStatus.Value())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(result.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result.Value.Details.CopyId);
    ret.Details.CopySource = std::move(result.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(result.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileToResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DownloadFileToResult> DataLakeFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto result = m_blobClient.AsBlockBlobClient().DownloadTo(fileName, options, context);
    Models::DownloadFileToResult ret;
    ret.ContentRange = std::move(result.Value.ContentRange);
    ret.FileSize = result.Value.BlobSize;
    ret.Details.HttpHeaders = FromBlobHttpHeaders(std::move(result.Value.Details.HttpHeaders));
    ret.Details.ETag = std::move(result.Value.Details.ETag);
    ret.Details.LastModified = std::move(result.Value.Details.LastModified);
    if (result.Value.Details.LeaseDuration.HasValue())
    {
      ret.Details.LeaseDuration
          = Models::LeaseDuration(result.Value.Details.LeaseDuration.Value().ToString());
    }
    ret.Details.LeaseState = result.Value.Details.LeaseState.HasValue()
        ? FromBlobLeaseState(result.Value.Details.LeaseState.Value())
        : ret.Details.LeaseState;
    ret.Details.LeaseStatus = result.Value.Details.LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result.Value.Details.LeaseStatus.Value())
        : ret.Details.LeaseStatus;
    ret.Details.Metadata = std::move(result.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(result.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(result.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(result.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(result.Value.Details.CopyId);
    ret.Details.CopySource = std::move(result.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(result.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(result.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(result.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(result.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(result.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(result.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(result.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(result.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = result.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileToResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::ScheduleFileDeletionResult> DataLakeFileClient::ScheduleDeletion(
      ScheduleFileExpiryOriginType expiryOrigin,
      const ScheduleFileDeletionOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::_detail::BlobRestClient::Blob::SetBlobExpiryOptions protocolLayerOptions;
    protocolLayerOptions.ExpiryOrigin = expiryOrigin;
    AZURE_ASSERT_MSG(
        !(options.ExpiresOn.HasValue() && options.TimeToExpire.HasValue()),
        "ExpiresOn and TimeToExpire are mutually exclusive.");

    if (options.ExpiresOn.HasValue())
    {
      protocolLayerOptions.ExpiryTime
          = options.ExpiresOn.Value().ToString(Azure::DateTime::DateFormat::Rfc1123);
    }
    else if (options.TimeToExpire.HasValue())
    {
      protocolLayerOptions.ExpiryTime = std::to_string(options.TimeToExpire.Value().count());
    }
    return Blobs::_detail::BlobRestClient::Blob::ScheduleDeletion(
        *m_pipeline, m_blobClient.m_blobUrl, protocolLayerOptions, context);
  }

}}}} // namespace Azure::Storage::Files::DataLake

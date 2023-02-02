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
    _detail::FileClient::AppendFileOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    if (options.TransactionalContentHash.HasValue())
    {
      if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Crc64)
      {
        protocolLayerOptions.TransactionalContentCrc64
            = options.TransactionalContentHash.Value().Value;
      }
      else if (options.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
      {
        protocolLayerOptions.TransactionalContentHash
            = options.TransactionalContentHash.Value().Value;
      }
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.Flush = options.Flush;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm.ToString();
    }
    protocolLayerOptions.LeaseAction = options.LeaseAction;
    protocolLayerOptions.ProposedLeaseId = options.LeaseId;
    if (options.LeaseDuration.HasValue())
    {
      protocolLayerOptions.LeaseDuration = static_cast<int64_t>(options.LeaseDuration->count());
    }
    return _detail::FileClient::Append(
        *m_pipeline, m_pathUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::FlushFileResult> DataLakeFileClient::Flush(
      int64_t position,
      const FlushFileOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::FileClient::FlushFileOptions protocolLayerOptions;
    protocolLayerOptions.Position = position;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    if (options.ContentHash.HasValue())
    {
      AZURE_ASSERT_MSG(
          options.ContentHash.Value().Algorithm == HashAlgorithm::Md5,
          "This operation only supports MD5 content hash.");
      protocolLayerOptions.ContentMD5 = options.ContentHash.Value().Value;
    }
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
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm.ToString();
    }
    protocolLayerOptions.LeaseAction = options.LeaseAction;
    protocolLayerOptions.ProposedLeaseId = options.LeaseId;
    if (options.LeaseDuration.HasValue())
    {
      protocolLayerOptions.LeaseDuration = static_cast<int64_t>(options.LeaseDuration->count());
    }
    return _detail::FileClient::Flush(*m_pipeline, m_pathUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::DeleteFileResult> DataLakeFileClient::Delete(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto response = DataLakePathClient::Delete(deleteOptions, context);
    Models::DeleteFileResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteFileResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::DeleteFileResult> DataLakeFileClient::DeleteIfExists(
      const DeleteFileOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    auto response = DataLakePathClient::DeleteIfExists(deleteOptions, context);
    Models::DeleteFileResult ret;
    ret.Deleted = response.Value.Deleted;
    return Azure::Response<Models::DeleteFileResult>(
        std::move(ret), std::move(response.RawResponse));
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
    auto response = m_blobClient.Download(blobOptions, context);
    Models::DownloadFileResult ret;
    ret.Body = std::move(response.Value.BodyStream);
    ret.Details.HttpHeaders = std::move(response.Value.Details.HttpHeaders);
    ret.ContentRange = std::move(response.Value.ContentRange);
    ret.FileSize = response.Value.BlobSize;
    ret.TransactionalContentHash = std::move(response.Value.TransactionalContentHash);
    ret.Details.ETag = std::move(response.Value.Details.ETag);
    ret.Details.LastModified = std::move(response.Value.Details.LastModified);
    ret.Details.LeaseDuration = std::move(response.Value.Details.LeaseDuration);
    if (response.Value.Details.LeaseState.HasValue())
    {
      ret.Details.LeaseState = std::move(response.Value.Details.LeaseState.Value());
    }
    if (response.Value.Details.LeaseStatus.HasValue())
    {
      ret.Details.LeaseStatus = std::move(response.Value.Details.LeaseStatus.Value());
    }
    ret.Details.Metadata = std::move(response.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(response.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(response.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(response.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(response.Value.Details.CopyId);
    ret.Details.CopySource = std::move(response.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(response.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(response.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(response.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(response.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(response.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(response.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(response.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(response.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = response.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileResult>(
        std::move(ret), std::move(response.RawResponse));
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
    blobOptions.HttpHeaders = options.HttpHeaders;
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
    blobOptions.HttpHeaders = options.HttpHeaders;
    blobOptions.Metadata = options.Metadata;
    return m_blobClient.AsBlockBlobClient().UploadFrom(buffer, bufferSize, blobOptions, context);
  }

  Azure::Response<Models::DownloadFileToResult> DataLakeFileClient::DownloadTo(
      uint8_t* buffer,
      size_t bufferSize,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto response
        = m_blobClient.AsBlockBlobClient().DownloadTo(buffer, bufferSize, options, context);
    Models::DownloadFileToResult ret;
    ret.ContentRange = std::move(response.Value.ContentRange);
    ret.FileSize = response.Value.BlobSize;
    ret.Details.HttpHeaders = std::move(response.Value.Details.HttpHeaders);
    ret.Details.ETag = std::move(response.Value.Details.ETag);
    ret.Details.LastModified = std::move(response.Value.Details.LastModified);
    ret.Details.LeaseDuration = std::move(response.Value.Details.LeaseDuration);
    if (response.Value.Details.LeaseState.HasValue())
    {
      ret.Details.LeaseState = response.Value.Details.LeaseState.Value();
    }
    if (response.Value.Details.LeaseStatus.HasValue())
    {
      ret.Details.LeaseStatus = response.Value.Details.LeaseStatus.Value();
    }
    ret.Details.Metadata = std::move(response.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(response.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(response.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(response.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(response.Value.Details.CopyId);
    ret.Details.CopySource = std::move(response.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(response.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(response.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(response.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(response.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(response.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(response.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(response.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(response.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = response.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileToResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::DownloadFileToResult> DataLakeFileClient::DownloadTo(
      const std::string& fileName,
      const DownloadFileToOptions& options,
      const Azure::Core::Context& context) const
  {
    auto response = m_blobClient.AsBlockBlobClient().DownloadTo(fileName, options, context);
    Models::DownloadFileToResult ret;
    ret.ContentRange = std::move(response.Value.ContentRange);
    ret.FileSize = response.Value.BlobSize;
    ret.Details.HttpHeaders = std::move(response.Value.Details.HttpHeaders);
    ret.Details.ETag = std::move(response.Value.Details.ETag);
    ret.Details.LastModified = std::move(response.Value.Details.LastModified);
    ret.Details.LeaseDuration = std::move(response.Value.Details.LeaseDuration);
    if (response.Value.Details.LeaseState.HasValue())
    {
      ret.Details.LeaseState = response.Value.Details.LeaseState.Value();
    }
    if (response.Value.Details.LeaseStatus.HasValue())
    {
      ret.Details.LeaseStatus = response.Value.Details.LeaseStatus.Value();
    }
    ret.Details.Metadata = std::move(response.Value.Details.Metadata);
    ret.Details.CreatedOn = std::move(response.Value.Details.CreatedOn);
    ret.Details.ExpiresOn = std::move(response.Value.Details.ExpiresOn);
    ret.Details.LastAccessedOn = std::move(response.Value.Details.LastAccessedOn);
    ret.Details.CopyId = std::move(response.Value.Details.CopyId);
    ret.Details.CopySource = std::move(response.Value.Details.CopySource);
    ret.Details.CopyStatus = std::move(response.Value.Details.CopyStatus);
    ret.Details.CopyStatusDescription = std::move(response.Value.Details.CopyStatusDescription);
    ret.Details.CopyProgress = std::move(response.Value.Details.CopyProgress);
    ret.Details.CopyCompletedOn = std::move(response.Value.Details.CopyCompletedOn);
    ret.Details.VersionId = std::move(response.Value.Details.VersionId);
    ret.Details.IsCurrentVersion = std::move(response.Value.Details.IsCurrentVersion);
    ret.Details.EncryptionKeySha256 = std::move(response.Value.Details.EncryptionKeySha256);
    ret.Details.EncryptionScope = std::move(response.Value.Details.EncryptionScope);
    ret.Details.IsServerEncrypted = response.Value.Details.IsServerEncrypted;
    return Azure::Response<Models::DownloadFileToResult>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::ScheduleFileDeletionResult> DataLakeFileClient::ScheduleDeletion(
      ScheduleFileExpiryOriginType expiryOrigin,
      const ScheduleFileDeletionOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::_detail::BlobClient::SetBlobExpiryOptions protocolLayerOptions;
    protocolLayerOptions.ExpiryOptions = expiryOrigin;
    AZURE_ASSERT_MSG(
        !(options.ExpiresOn.HasValue() && options.TimeToExpire.HasValue()),
        "ExpiresOn and TimeToExpire are mutually exclusive.");

    if (options.ExpiresOn.HasValue())
    {
      protocolLayerOptions.ExpiresOn
          = options.ExpiresOn.Value().ToString(Azure::DateTime::DateFormat::Rfc1123);
    }
    else if (options.TimeToExpire.HasValue())
    {
      protocolLayerOptions.ExpiresOn = std::to_string(options.TimeToExpire.Value().count());
    }
    return Blobs::_detail::BlobClient::SetExpiry(
        *m_pipeline, m_blobClient.m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::QueryFileResult> DataLakeFileClient::Query(
      const std::string& querySqlExpression,
      const QueryFileOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::QueryBlobOptions blobOptions;
    blobOptions.InputTextConfiguration = options.InputTextConfiguration;
    blobOptions.OutputTextConfiguration = options.OutputTextConfiguration;
    blobOptions.ErrorHandler = options.ErrorHandler;
    blobOptions.ProgressHandler = options.ProgressHandler;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto response
        = m_blobClient.AsBlockBlobClient().Query(querySqlExpression, blobOptions, context);
    Models::QueryFileResult ret;
    ret.BodyStream = std::move(response.Value.BodyStream);
    ret.ETag = std::move(response.Value.ETag);
    ret.LastModified = std::move(response.Value.LastModified);
    ret.LeaseDuration = std::move(response.Value.LeaseDuration);
    ret.LeaseState = std::move(response.Value.LeaseState);
    ret.LeaseStatus = std::move(response.Value.LeaseStatus);
    ret.IsServerEncrypted = response.Value.IsServerEncrypted;
    return Azure::Response<Models::QueryFileResult>(
        std::move(ret), std::move(response.RawResponse));
  }

}}}} // namespace Azure::Storage::Files::DataLake

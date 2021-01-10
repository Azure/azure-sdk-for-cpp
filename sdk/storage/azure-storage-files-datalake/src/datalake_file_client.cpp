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

  FileClient FileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& filePath,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    fileUri.AppendPath(Storage::Details::UrlEncodePath(filePath));

    if (parsedConnectionString.KeyCredential)
    {
      return FileClient(fileUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return FileClient(fileUri.GetAbsoluteUrl(), options);
    }
  }

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : PathClient(fileUri, credential, options),
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

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : PathClient(fileUri, credential, options),
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

  FileClient::FileClient(const std::string& fileUri, const DataLakeClientOptions& options)
      : PathClient(fileUri, options), m_blockBlobClient(m_blobClient.AsBlockBlobClient())
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<Models::AppendFileDataResult> FileClient::AppendData(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const AppendFileDataOptions& options) const
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
        m_dfsUri, *content, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::FlushFileDataResult> FileClient::FlushData(
      int64_t endingOffset,
      const FlushFileDataOptions& options) const
  {
    Details::DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = endingOffset;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    if (options.ContentHash.HasValue()
        && options.ContentHash.GetValue().Algorithm != HashAlgorithm::Md5)
    {
      abort();
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
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::RenameFileResult> FileClient::Rename(
      const std::string& destinationPath,
      const RenameFileOptions& options) const
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_dfsUri.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUri = m_dfsUri;
    destinationDfsUri.SetPath(destinationFileSystem.GetValue() + '/' + destinationPath);

    Details::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
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
    auto result = Details::DataLakeRestClient::Path::Create(
        destinationDfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    auto ret = Models::RenameFileResult();
    return Azure::Core::Response<Models::RenameFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteFileResult> FileClient::Delete(
      const FileDeleteOptions& options) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    auto result = PathClient::Delete(deleteOptions);
    auto ret = Models::DeleteFileResult();
    ret.Deleted = true;
    return Azure::Core::Response<Models::DeleteFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteFileResult> FileClient::DeleteIfExists(
      const FileDeleteOptions& options) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    auto result = PathClient::DeleteIfExists(deleteOptions);
    auto ret = Models::DeleteFileResult();
    ret.Deleted = result->Deleted;
    return Azure::Core::Response<Models::DeleteFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ReadFileResult> FileClient::Read(
      const ReadFileOptions& options) const
  {
    Blobs::DownloadBlobOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Range = options.Range;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.Download(blobOptions);
    Models::ReadFileResult ret;
    ret.Body = std::move(result->BodyStream);
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.ContentRange = std::move(result->ContentRange);
    ret.TransactionalContentHash = std::move(result->TransactionalContentHash);
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.LeaseDuration = std::move(result->LeaseDuration);
    ret.LeaseState = result->LeaseState.HasValue()
        ? FromBlobLeaseState(result->LeaseState.GetValue())
        : ret.LeaseState;
    ret.LeaseStatus = result->LeaseStatus.HasValue()
        ? FromBlobLeaseStatus(result->LeaseStatus.GetValue())
        : ret.LeaseStatus;
    ret.Metadata = std::move(result->Metadata);
    ret.CreatedOn = std::move(result->CreatedOn);
    ret.ExpiresOn = std::move(result->ExpiriesOn);
    ret.LastAccessedOn = std::move(result->LastAccessedOn);
    return Azure::Core::Response<Models::ReadFileResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::UploadFileFromResult> FileClient::UploadFrom(
      const std::string& fileName,
      const UploadFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFrom(fileName, blobOptions);
  }

  Azure::Core::Response<Models::UploadFileFromResult> FileClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromPathHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFrom(buffer, bufferSize, blobOptions);
  }

  Azure::Core::Response<Models::DownloadFileToResult> FileClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(buffer, bufferSize, options);
    Models::DownloadFileToResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = result->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    return Azure::Core::Response<Models::DownloadFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DownloadFileToResult> FileClient::DownloadTo(
      const std::string& fileName,
      const DownloadFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(fileName, options);
    Models::DownloadFileToResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = result->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    return Azure::Core::Response<Models::DownloadFileToResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ScheduleFileDeletionResult> FileClient::ScheduleDeletion(
      ScheduleFileExpiryOriginType expiryOrigin,
      const ScheduleFileDeletionOptions& options) const
  {
    Blobs::Details::BlobRestClient::Blob::SetBlobExpiryOptions protocolLayerOptions;
    protocolLayerOptions.ExpiryOrigin = expiryOrigin;
    if (options.ExpiresOn.HasValue() && options.TimeToExpireInMs.HasValue())
    {
      throw std::runtime_error("ExpiresOn and TimeToExpireInMs should be mutually exlusive.");
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

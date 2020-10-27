// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

#include <limits>
#include <utility> //std::pair

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace {
    std::pair<int64_t, int64_t> GetOffsetLength(const std::string& rangeString)
    {
      int64_t offset = std::numeric_limits<int64_t>::max();
      int64_t length = std::numeric_limits<int64_t>::max();
      const std::string c_bytesPrefix = "bytes ";
      if (rangeString.length() > c_bytesPrefix.length())
      {
        auto subRangeString = rangeString.substr(c_bytesPrefix.length());
        std::string::const_iterator cur = subRangeString.begin();
        offset = std::stoll(Details::GetSubstringTillDelimiter('-', subRangeString, cur));
        if (cur != subRangeString.end())
        {
          length = std::stoll(Details::GetSubstringTillDelimiter('/', subRangeString, cur)) - offset
              + 1;
        }
        else
        {
          throw std::runtime_error("The format of the range string is not correct: " + rangeString);
        }
      }
      return std::make_pair(offset, length);
    }

    DataLakeHttpHeaders FromBlobHttpHeaders(Blobs::BlobHttpHeaders headers)
    {
      DataLakeHttpHeaders ret;
      ret.CacheControl = std::move(headers.CacheControl);
      ret.ContentDisposition = std::move(headers.ContentDisposition);
      ret.ContentEncoding = std::move(headers.ContentEncoding);
      ret.ContentLanguage = std::move(headers.ContentLanguage);
      ret.ContentType = std::move(headers.ContentType);
      return ret;
    }

    Blobs::BlobHttpHeaders FromDataLakeHttpHeaders(DataLakeHttpHeaders headers)
    {
      Blobs::BlobHttpHeaders ret;
      ret.CacheControl = std::move(headers.CacheControl);
      ret.ContentDisposition = std::move(headers.ContentDisposition);
      ret.ContentEncoding = std::move(headers.ContentEncoding);
      ret.ContentLanguage = std::move(headers.ContentLanguage);
      ret.ContentType = std::move(headers.ContentType);
      return ret;
    }

    LeaseStateType FromBlobLeaseState(Blobs::BlobLeaseState state)
    {
      switch (state)
      {
        case Blobs::BlobLeaseState::Available:
          return LeaseStateType::Available;
        case Blobs::BlobLeaseState::Breaking:
          return LeaseStateType::Breaking;
        case Blobs::BlobLeaseState::Broken:
          return LeaseStateType::Broken;
        case Blobs::BlobLeaseState::Expired:
          return LeaseStateType::Expired;
        case Blobs::BlobLeaseState::Leased:
          return LeaseStateType::Leased;
        default:
          return LeaseStateType::Unknown;
      }
    }

    LeaseStatusType FromBlobLeaseStatus(Blobs::BlobLeaseStatus status)
    {
      switch (status)
      {
        case Blobs::BlobLeaseStatus::Locked:
          return LeaseStatusType::Locked;
        case Blobs::BlobLeaseStatus::Unlocked:
          return LeaseStatusType::Unlocked;
        default:
          return LeaseStatusType::Unknown;
      }
    }
  } // namespace

  FileClient FileClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& filePath,
      const FileClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileUri = std::move(parsedConnectionString.DataLakeServiceUri);
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
      std::shared_ptr<SharedKeyCredential> credential,
      const FileClientOptions& options)
      : PathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const FileClientOptions& options)
      : PathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient::FileClient(const std::string& fileUri, const FileClientOptions& options)
      : PathClient(fileUri, options), m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<AppendFileDataResult> FileClient::AppendData(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const AppendFileDataOptions& options) const
  {
    DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = content->Length();
    protocolLayerOptions.TransactionalContentMd5 = options.ContentMd5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return DataLakeRestClient::Path::AppendData(
        m_dfsUri, *content, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<FlushFileDataResult> FileClient::FlushData(
      int64_t endingOffset,
      const FlushFileDataOptions& options) const
  {
    DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = endingOffset;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.ContentMd5 = options.ContentMd5;
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
    return DataLakeRestClient::Path::FlushData(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<RenameFileResult> FileClient::Rename(
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

    DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
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
    auto result = DataLakeRestClient::Path::Create(
        destinationDfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    auto ret = RenameFileResult();
    return Azure::Core::Response<RenameFileResult>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<DeleteFileResult> FileClient::Delete(const FileDeleteOptions& options) const
  {
    DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = DataLakeRestClient::Path::Delete(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    auto ret = DeleteFileResult();
    return Azure::Core::Response<DeleteFileResult>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<ReadFileResult> FileClient::Read(const ReadFileOptions& options) const
  {
    Blobs::DownloadBlobOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Offset = options.Offset;
    blobOptions.Length = options.Length;
    blobOptions.AccessConditions.IfMatch = options.AccessConditions.IfMatch;
    blobOptions.AccessConditions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobClient.Download(blobOptions);
    ReadFileResult ret;
    ret.Body = std::move(result->BodyStream);
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    Azure::Core::Nullable<int64_t> RangeOffset;
    Azure::Core::Nullable<int64_t> RangeLength;
    if (result->ContentRange.HasValue())
    {
      auto range = GetOffsetLength(result->ContentRange.GetValue());
      RangeOffset = range.first;
      RangeLength = range.second;
    }
    ret.RangeOffset = RangeOffset;
    ret.RangeLength = RangeLength;
    ret.TransactionalMd5 = std::move(result->TransactionalContentMd5);
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
    ret.CreationTime = std::move(result->CreationTime);
    ret.ExpiryTime = std::move(result->ExpiryTime);
    ret.LastAccessTime = std::move(result->LastAccessTime);
    return Azure::Core::Response<ReadFileResult>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<UploadFileFromResult> FileClient::UploadFrom(
      const std::string& file,
      const UploadFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromDataLakeHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFrom(file, blobOptions);
  }

  Azure::Core::Response<UploadFileFromResult> FileClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadFileFromOptions& options) const
  {
    Blobs::UploadBlockBlobFromOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromDataLakeHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFrom(buffer, bufferSize, blobOptions);
  }

  Azure::Core::Response<DownloadFileToResult> FileClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(buffer, bufferSize, options);
    DownloadFileToResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = std::move(result->ServerEncrypted);
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    return Azure::Core::Response<DownloadFileToResult>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<DownloadFileToResult> FileClient::DownloadTo(
      const std::string& file,
      const DownloadFileToOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadTo(file, options);
    DownloadFileToResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = std::move(result->ServerEncrypted);
    ret.EncryptionKeySha256 = std::move(result->EncryptionKeySha256);
    return Azure::Core::Response<DownloadFileToResult>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<ScheduleFileDeletionResult> FileClient::ScheduleDeletion(
      ScheduleFileExpiryOriginType expiryOrigin,
      const ScheduleFileDeletionOptions& options) const
  {
    Blobs::BlobRestClient::Blob::SetBlobExpiryOptions protocolLayerOptions;
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
    return Blobs::BlobRestClient::Blob::ScheduleDeletion(
        options.Context, *m_pipeline, m_blobClient.m_blobUrl, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

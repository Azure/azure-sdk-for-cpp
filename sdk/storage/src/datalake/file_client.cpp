// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/file_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/constants.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "datalake/datalake_utilities.hpp"
#include "http/curl/curl.hpp"

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
      auto ret = LeaseStateType::Unknown;
      switch (state)
      {
        case Blobs::BlobLeaseState::Available:
          ret = LeaseStateType::Available;
          break;
        case Blobs::BlobLeaseState::Breaking:
          ret = LeaseStateType::Breaking;
          break;
        case Blobs::BlobLeaseState::Broken:
          ret = LeaseStateType::Broken;
          break;
        case Blobs::BlobLeaseState::Expired:
          ret = LeaseStateType::Expired;
          break;
        case Blobs::BlobLeaseState::Leased:
          ret = LeaseStateType::Leased;
          break;
        default:
          ret = LeaseStateType::Unknown;
          break;
      }
      return ret;
    }

    LeaseStatusType FromBlobLeaseStatus(Blobs::BlobLeaseStatus status)
    {
      auto ret = LeaseStatusType::Unknown;
      switch (status)
      {
        case Blobs::BlobLeaseStatus::Locked:
          ret = LeaseStatusType::Locked;
          break;
        case Blobs::BlobLeaseStatus::Unlocked:
          ret = LeaseStatusType::Unlocked;
          break;
        default:
          ret = LeaseStatusType::Unknown;
          break;
      }
      return ret;
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
    fileUri.AppendPath(fileSystemName, true);
    fileUri.AppendPath(filePath, true);

    if (parsedConnectionString.KeyCredential)
    {
      return FileClient(fileUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return FileClient(fileUri.ToString(), options);
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
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
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

  FileClient::FileClient(
      const std::string& fileUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const FileClientOptions& options)
      : PathClient(fileUri, credential, options),
        m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(
        std::make_unique<Core::Credentials::Policy::BearerTokenAuthenticationPolicy>(
            credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient::FileClient(const std::string& fileUri, const FileClientOptions& options)
      : PathClient(fileUri, options), m_blockBlobClient(m_blobClient.GetBlockBlobClient())
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<PathAppendDataResponse> FileClient::AppendData(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const PathAppendDataOptions& options) const
  {
    DataLakeRestClient::Path::AppendDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = offset;
    protocolLayerOptions.ContentLength = content->Length();
    protocolLayerOptions.TransactionalContentMD5 = options.ContentMD5;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    return DataLakeRestClient::Path::AppendData(
        m_dfsUri.ToString(), *content, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<PathFlushDataResponse> FileClient::FlushData(
      int64_t endingOffset,
      const PathFlushDataOptions& options) const
  {
    DataLakeRestClient::Path::FlushDataOptions protocolLayerOptions;
    protocolLayerOptions.Position = endingOffset;
    protocolLayerOptions.RetainUncommittedData = options.RetainUncommittedData;
    protocolLayerOptions.Close = options.Close;
    protocolLayerOptions.ContentLength = 0;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
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
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<FileRenameInfo> FileClient::Rename(
      const std::string& destinationPath,
      const FileRenameOptions& options)
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
        destinationDfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    m_dfsUri = std::move(destinationDfsUri);
    m_blobClient = Blobs::BlobClient(
        UriBuilder(Details::GetBlobUriFromUri(m_dfsUri.ToString())), m_pipeline);
    m_blockBlobClient = Blobs::BlockBlobClient(m_blobClient);
    auto ret = FileRenameInfo();
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<FileRenameInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileDeleteInfo> FileClient::Delete(const FileDeleteOptions& options) const
  {
    DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = DataLakeRestClient::Path::Delete(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    auto ret = FileDeleteInfo();
    return Azure::Core::Response<FileDeleteInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileReadInfo> FileClient::Read(const FileReadOptions& options) const
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
    FileReadInfo ret;
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
    ret.TransactionalMD5 = std::move(result->ContentMD5);
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
    return Azure::Core::Response<FileReadInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileContentInfo> FileClient::UploadFromFile(
      const std::string& file,
      const UploadFileOptions& options) const
  {
    Blobs::UploadBlobOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromDataLakeHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFromFile(file, blobOptions);
  }

  Azure::Core::Response<FileContentInfo> FileClient::UploadFromBuffer(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadFileOptions& options) const
  {
    Blobs::UploadBlobOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.ChunkSize = options.ChunkSize;
    blobOptions.HttpHeaders = FromDataLakeHttpHeaders(options.HttpHeaders);
    blobOptions.Metadata = options.Metadata;
    blobOptions.Concurrency = options.Concurrency;
    return m_blockBlobClient.UploadFromBuffer(buffer, bufferSize, blobOptions);
  }

  Azure::Core::Response<FileDownloadInfo> FileClient::DownloadToBuffer(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadFileOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadToBuffer(buffer, bufferSize, options);
    FileDownloadInfo ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = std::move(result->ServerEncrypted);
    ret.EncryptionKeySHA256 = std::move(result->EncryptionKeySHA256);
    return Azure::Core::Response<FileDownloadInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileDownloadInfo> FileClient::DownloadToFile(
      const std::string& file,
      const DownloadFileOptions& options) const
  {
    auto result = m_blockBlobClient.DownloadToFile(file, options);
    FileDownloadInfo ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.ContentLength = result->ContentLength;
    ret.HttpHeaders = FromBlobHttpHeaders(std::move(result->HttpHeaders));
    ret.Metadata = std::move(result->Metadata);
    ret.ServerEncrypted = std::move(result->ServerEncrypted);
    ret.EncryptionKeySHA256 = std::move(result->EncryptionKeySHA256);
    return Azure::Core::Response<FileDownloadInfo>(std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake

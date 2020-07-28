// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_client.hpp"

#include "blobs/append_blob_client.hpp"
#include "blobs/block_blob_client.hpp"
#include "blobs/page_blob_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/concurrent_transfer.hpp"
#include "common/constants.hpp"
#include "common/file_io.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/storage_version.hpp"
#include "credentials/policy/policies.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient BlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Details::ParseConnectionString(connectionString);
    auto blobUri = std::move(parsedConnectionString.BlobServiceUri);
    blobUri.AppendPath(containerName, true);
    blobUri.AppendPath(blobName, true);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobClient(blobUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobClient(blobUri.ToString(), options);
    }
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, BlobServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, BlobServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(
        std::make_unique<Core::Credentials::Policy::BearerTokenAuthenticationPolicy>(
            credential, Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(const std::string& blobUri, const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, BlobServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlockBlobClient BlobClient::GetBlockBlobClient() const { return BlockBlobClient(*this); }

  AppendBlobClient BlobClient::GetAppendBlobClient() const { return AppendBlobClient(*this); }

  PageBlobClient BlobClient::GetPageBlobClient() const { return PageBlobClient(*this); }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQuery(Details::c_HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQuery(Details::c_HttpQuerySnapshot, snapshot);
    }
    return newClient;
  }

  Azure::Core::Response<BlobDownloadResponse> BlobClient::Download(
      const DownloadBlobOptions& options) const
  {
    BlobRestClient::Blob::DownloadOptions protocolLayerOptions;
    if (options.Offset.HasValue() && options.Length.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(), options.Offset.GetValue() + options.Length.GetValue() - 1);
    }
    else if (options.Offset.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(),
          std::numeric_limits<std::remove_reference_t<decltype(options.Offset.GetValue())>>::max());
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;

    return BlobRestClient::Blob::Download(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<BlobDownloadInfo> BlobClient::DownloadToBuffer(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadBlobToBufferOptions& options) const
  {
    constexpr int64_t c_defaultChunkSize = 4 * 1024 * 1024;

    // Just start downloading using an initial chunk. If it's a small blob, we'll get the whole
    // thing in one shot. If it's a large blob, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Offset.HasValue() ? options.Offset.GetValue() : 0;
    int64_t firstChunkLength = c_defaultChunkSize;
    if (options.InitialChunkSize.HasValue())
    {
      firstChunkLength = options.InitialChunkSize.GetValue();
    }
    if (options.Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Length.GetValue());
    }

    DownloadBlobOptions firstChunkOptions;
    firstChunkOptions.Context = options.Context;
    firstChunkOptions.Offset = options.Offset;
    if (firstChunkOptions.Offset.HasValue())
    {
      firstChunkOptions.Length = firstChunkLength;
    }

    auto firstChunk = Download(firstChunkOptions);

    int64_t blobSize;
    int64_t blobRangeSize;
    if (firstChunkOptions.Offset.HasValue())
    {
      blobSize = std::stoll(firstChunk->ContentRange.GetValue().substr(
          firstChunk->ContentRange.GetValue().find('/') + 1));
      blobRangeSize = blobSize - firstChunkOffset;
      if (options.Length.HasValue())
      {
        blobRangeSize = std::min(blobRangeSize, options.Length.GetValue());
      }
    }
    else
    {
      blobSize = firstChunk->BodyStream->Length();
      blobRangeSize = blobSize;
    }
    firstChunkLength = std::min(firstChunkLength, blobRangeSize);

    if (static_cast<std::size_t>(blobRangeSize) > bufferSize)
    {
      throw std::runtime_error(
          "buffer is not big enough, blob range size is " + std::to_string(blobRangeSize));
    }

    int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
        firstChunkOptions.Context, *(firstChunk->BodyStream), buffer, firstChunkLength);
    if (bytesRead != firstChunkLength)
    {
      throw std::runtime_error("error when reading body stream");
    }
    firstChunk->BodyStream.reset();

    auto returnTypeConverter = [](Azure::Core::Response<BlobDownloadResponse>& response) {
      BlobDownloadInfo ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.HttpHeaders = std::move(response->HttpHeaders);
      ret.Metadata = std::move(response->Metadata);
      ret.BlobType = response->BlobType;
      ret.ServerEncrypted = response->ServerEncrypted;
      ret.EncryptionKeySHA256 = std::move(response->EncryptionKeySHA256);
      return Azure::Core::Response<BlobDownloadInfo>(
          std::move(ret),
          std::make_unique<Azure::Core::Http::RawResponse>(std::move(response.GetRawResponse())));
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadBlobOptions chunkOptions;
            chunkOptions.Context = options.Context;
            chunkOptions.Offset = offset;
            chunkOptions.Length = length;
            auto chunk = Download(chunkOptions);
            int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
                chunkOptions.Context,
                *(chunk->BodyStream),
                buffer + (offset - firstChunkOffset),
                chunkOptions.Length.GetValue());
            if (bytesRead != chunkOptions.Length.GetValue())
            {
              throw std::runtime_error("error when reading body stream");
            }

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = blobRangeSize - firstChunkLength;
    int64_t chunkSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t c_grainSize = 4 * 1024;
      chunkSize = remainingSize / options.Concurrency;
      chunkSize = (std::max(chunkSize, int64_t(1)) + c_grainSize - 1) / c_grainSize * c_grainSize;
      chunkSize = std::min(chunkSize, c_defaultChunkSize);
    }

    Details::ConcurrentTransfer(
        remainingOffset, remainingSize, chunkSize, options.Concurrency, downloadChunkFunc);
    ret->ContentLength = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<BlobDownloadInfo> BlobClient::DownloadToFile(
      const std::string& file,
      const DownloadBlobToFileOptions& options) const
  {
    constexpr int64_t c_defaultChunkSize = 4 * 1024 * 1024;

    // Just start downloading using an initial chunk. If it's a small blob, we'll get the whole
    // thing in one shot. If it's a large blob, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    int64_t firstChunkOffset = options.Offset.HasValue() ? options.Offset.GetValue() : 0;
    int64_t firstChunkLength = c_defaultChunkSize;
    if (options.InitialChunkSize.HasValue())
    {
      firstChunkLength = options.InitialChunkSize.GetValue();
    }
    if (options.Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Length.GetValue());
    }

    DownloadBlobOptions firstChunkOptions;
    firstChunkOptions.Context = options.Context;
    firstChunkOptions.Offset = options.Offset;
    if (firstChunkOptions.Offset.HasValue())
    {
      firstChunkOptions.Length = firstChunkLength;
    }

    Details::FileWriter fileWriter(file);

    auto firstChunk = Download(firstChunkOptions);

    int64_t blobSize;
    int64_t blobRangeSize;
    if (firstChunkOptions.Offset.HasValue())
    {
      blobSize = std::stoll(firstChunk->ContentRange.GetValue().substr(
          firstChunk->ContentRange.GetValue().find('/') + 1));
      blobRangeSize = blobSize - firstChunkOffset;
      if (options.Length.HasValue())
      {
        blobRangeSize = std::min(blobRangeSize, options.Length.GetValue());
      }
    }
    else
    {
      blobSize = firstChunk->BodyStream->Length();
      blobRangeSize = blobSize;
    }
    firstChunkLength = std::min(firstChunkLength, blobRangeSize);

    auto bodyStreamToFile = [](Azure::Core::Http::BodyStream& stream,
                               Details::FileWriter& fileWriter,
                               int64_t offset,
                               int64_t length,
                               Azure::Core::Context& context) {
      constexpr std::size_t bufferSize = 4 * 1024 * 1024;
      std::vector<uint8_t> buffer(bufferSize);
      while (length > 0)
      {
        int64_t readSize = std::min(static_cast<int64_t>(bufferSize), length);
        int64_t bytesRead
            = Azure::Core::Http::BodyStream::ReadToCount(context, stream, buffer.data(), readSize);
        if (bytesRead != readSize)
        {
          throw std::runtime_error("error when reading body stream");
        }
        fileWriter.Write(buffer.data(), bytesRead, offset);
        length -= bytesRead;
        offset += bytesRead;
      }
    };

    bodyStreamToFile(
        *(firstChunk->BodyStream), fileWriter, 0, firstChunkLength, firstChunkOptions.Context);
    firstChunk->BodyStream.reset();

    auto returnTypeConverter = [](Azure::Core::Response<BlobDownloadResponse>& response) {
      BlobDownloadInfo ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.HttpHeaders = std::move(response->HttpHeaders);
      ret.Metadata = std::move(response->Metadata);
      ret.BlobType = response->BlobType;
      ret.ServerEncrypted = response->ServerEncrypted;
      ret.EncryptionKeySHA256 = std::move(response->EncryptionKeySHA256);
      return Azure::Core::Response<BlobDownloadInfo>(
          std::move(ret),
          std::make_unique<Azure::Core::Http::RawResponse>(std::move(response.GetRawResponse())));
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadBlobOptions chunkOptions;
            chunkOptions.Context = options.Context;
            chunkOptions.Offset = offset;
            chunkOptions.Length = length;
            auto chunk = Download(chunkOptions);
            bodyStreamToFile(
                *(chunk->BodyStream),
                fileWriter,
                offset - firstChunkOffset,
                chunkOptions.Length.GetValue(),
                chunkOptions.Context);

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = blobRangeSize - firstChunkLength;
    int64_t chunkSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t c_grainSize = 4 * 1024;
      chunkSize = remainingSize / options.Concurrency;
      chunkSize = (std::max(chunkSize, int64_t(1)) + c_grainSize - 1) / c_grainSize * c_grainSize;
      chunkSize = std::min(chunkSize, c_defaultChunkSize);
    }

    Details::ConcurrentTransfer(
        remainingOffset, remainingSize, chunkSize, options.Concurrency, downloadChunkFunc);
    ret->ContentLength = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<BlobProperties> BlobClient::GetProperties(
      const GetBlobPropertiesOptions& options) const
  {
    BlobRestClient::Blob::GetPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::Blob::GetProperties(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<BlobInfo> BlobClient::SetHttpHeaders(
      BlobHttpHeaders httpHeaders,
      const SetBlobHttpHeadersOptions& options) const
  {
    BlobRestClient::Blob::SetHttpHeadersOptions protocolLayerOptions;
    protocolLayerOptions.HttpHeaders = std::move(httpHeaders);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::Blob::SetHttpHeaders(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<BlobInfo> BlobClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetBlobMetadataOptions& options) const
  {
    BlobRestClient::Blob::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::Blob::SetMetadata(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<SetBlobAccessTierInfo> BlobClient::SetAccessTier(
      AccessTier Tier,
      const SetAccessTierOptions& options) const
  {
    BlobRestClient::Blob::SetAccessTierOptions protocolLayerOptions;
    protocolLayerOptions.Tier = Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    return BlobRestClient::Blob::SetAccessTier(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<BlobCopyInfo> BlobClient::StartCopyFromUri(
      const std::string& sourceUri,
      const StartCopyFromUriOptions& options) const
  {
    BlobRestClient::Blob::StartCopyFromUriOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceLeaseId = options.SourceConditions.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceConditions.IfNoneMatch;
    return BlobRestClient::Blob::StartCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<AbortCopyBlobInfo> BlobClient::AbortCopyFromUri(
      const std::string& copyId,
      const AbortCopyFromUriOptions& options) const
  {
    BlobRestClient::Blob::AbortCopyFromUriOptions protocolLayerOptions;
    protocolLayerOptions.CopyId = copyId;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return BlobRestClient::Blob::AbortCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<BlobSnapshotInfo> BlobClient::CreateSnapshot(
      const CreateSnapshotOptions& options) const
  {
    BlobRestClient::Blob::CreateSnapshotOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::Blob::CreateSnapshot(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<DeleteBlobInfo> BlobClient::Delete(const DeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::Blob::Delete(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<UndeleteBlobInfo> BlobClient::Undelete(
      const UndeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::UndeleteOptions protocolLayerOptions;
    return BlobRestClient::Blob::Undelete(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

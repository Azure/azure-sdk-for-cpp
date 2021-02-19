// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/reliable_stream.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"
#include "azure/storage/blobs/version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient BlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::Details::ParseConnectionString(connectionString);
    auto blobUrl = std::move(parsedConnectionString.BlobServiceUrl);
    blobUrl.AppendPath(Storage::Details::UrlEncodePath(blobContainerName));
    blobUrl.AppendPath(Storage::Details::UrlEncodePath(blobName));

    if (parsedConnectionString.KeyCredential)
    {
      return BlobClient(blobUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobClient(blobUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobClient::BlobClient(
      const std::string& blobUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, options)
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

  BlobClient::BlobClient(
      const std::string& blobUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, options)
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

  BlobClient::BlobClient(const std::string& blobUrl, const BlobClientOptions& options)
      : m_blobUrl(blobUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
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

  BlockBlobClient BlobClient::AsBlockBlobClient() const { return BlockBlobClient(*this); }

  AppendBlobClient BlobClient::AsAppendBlobClient() const { return AppendBlobClient(*this); }

  PageBlobClient BlobClient::AsPageBlobClient() const { return PageBlobClient(*this); }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::HttpQuerySnapshot, Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlobClient BlobClient::WithVersionId(const std::string& versionId) const
  {
    BlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::HttpQueryVersionId,
          Storage::Details::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Core::Response<Models::DownloadBlobResult> BlobClient::Download(
      const DownloadBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::DownloadBlobOptions protocolLayerOptions;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.RangeHashAlgorithm = options.RangeHashAlgorithm;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }

    auto downloadResponse = Details::BlobRestClient::Blob::Download(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);

    {
      // In case network failure during reading the body
      const Azure::Core::ETag eTag = downloadResponse->Details.ETag;

      auto retryFunction
          = [this, options, eTag](
                const Azure::Core::Context& context,
                const HttpGetterInfo& retryInfo) -> std::unique_ptr<Azure::Core::Http::BodyStream> {
        DownloadBlobOptions newOptions = options;
        newOptions.Range = Core::Http::Range();
        newOptions.Range.GetValue().Offset
            = (options.Range.HasValue() ? options.Range.GetValue().Offset : 0) + retryInfo.Offset;
        if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
        {
          newOptions.Range.GetValue().Length
              = options.Range.GetValue().Length.GetValue() - retryInfo.Offset;
        }
        if (!newOptions.AccessConditions.IfMatch.HasValue())
        {
          newOptions.AccessConditions.IfMatch = eTag;
        }
        return std::move(Download(newOptions, context)->BodyStream);
      };

      ReliableStreamOptions reliableStreamOptions;
      reliableStreamOptions.MaxRetryRequests = Storage::Details::ReliableStreamRetryCount;
      downloadResponse->BodyStream = std::make_unique<ReliableStream>(
          std::move(downloadResponse->BodyStream), reliableStreamOptions, retryFunction);
    }
    if (downloadResponse->BlobType == Models::BlobType::AppendBlob
        && !downloadResponse->Details.IsSealed.HasValue())
    {
      downloadResponse->Details.IsSealed = false;
    }
    if (downloadResponse->Details.VersionId.HasValue()
        && !downloadResponse->Details.IsCurrentVersion.HasValue())
    {
      downloadResponse->Details.IsCurrentVersion = false;
    }
    return downloadResponse;
  }

  Azure::Core::Response<Models::DownloadBlobToResult> BlobClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadBlobToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small blob, we'll get the whole
    // thing in one shot. If it's a large blob, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    const int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.GetValue().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;
    if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.GetValue().Length.GetValue());
    }

    DownloadBlobOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.GetValue().Length = firstChunkLength;
    }

    auto firstChunk = Download(firstChunkOptions, context);
    const Azure::Core::ETag eTag = firstChunk->Details.ETag;

    const int64_t blobSize = firstChunk->BlobSize;
    int64_t blobRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      blobRangeSize = blobSize - firstChunkOffset;
      if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
      {
        blobRangeSize = std::min(blobRangeSize, options.Range.GetValue().Length.GetValue());
      }
    }
    else
    {
      blobRangeSize = blobSize;
    }
    firstChunkLength = std::min(firstChunkLength, blobRangeSize);

    if (static_cast<std::size_t>(blobRangeSize) > bufferSize)
    {
      throw Azure::Core::RequestFailedException(
          "buffer is not big enough, blob range size is " + std::to_string(blobRangeSize));
    }

    int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
        context, *(firstChunk->BodyStream), buffer, firstChunkLength);
    if (bytesRead != firstChunkLength)
    {
      throw Azure::Core::RequestFailedException("error when reading body stream");
    }
    firstChunk->BodyStream.reset();

    auto returnTypeConverter = [](Azure::Core::Response<Models::DownloadBlobResult>& response) {
      Models::DownloadBlobToResult ret;
      ret.BlobType = std::move(response->BlobType);
      ret.ContentRange = std::move(response->ContentRange);
      ret.BlobSize = response->BlobSize;
      ret.TransactionalContentHash = std::move(response->TransactionalContentHash);
      ret.Details = std::move(response->Details);
      return Azure::Core::Response<Models::DownloadBlobToResult>(
          std::move(ret), response.ExtractRawResponse());
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadBlobOptions chunkOptions;
            chunkOptions.Range = Core::Http::Range();
            chunkOptions.Range.GetValue().Offset = offset;
            chunkOptions.Range.GetValue().Length = length;
            if (!chunkOptions.AccessConditions.IfMatch.HasValue())
            {
              chunkOptions.AccessConditions.IfMatch = eTag;
            }
            auto chunk = Download(chunkOptions, context);
            int64_t bytesRead = Azure::Core::Http::BodyStream::ReadToCount(
                context,
                *(chunk->BodyStream),
                buffer + (offset - firstChunkOffset),
                chunkOptions.Range.GetValue().Length.GetValue());
            if (bytesRead != chunkOptions.Range.GetValue().Length.GetValue())
            {
              throw Azure::Core::RequestFailedException("error when reading body stream");
            }

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
              ret->TransactionalContentHash.Reset();
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = blobRangeSize - firstChunkLength;

    Storage::Details::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret->ContentRange.Offset = firstChunkOffset;
    ret->ContentRange.Length = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::DownloadBlobToResult> BlobClient::DownloadTo(
      const std::string& fileName,
      const DownloadBlobToOptions& options,
      const Azure::Core::Context& context) const
  {
    // Just start downloading using an initial chunk. If it's a small blob, we'll get the whole
    // thing in one shot. If it's a large blob, we'll get its full size in Content-Range and can
    // keep downloading it in chunks.
    const int64_t firstChunkOffset = options.Range.HasValue() ? options.Range.GetValue().Offset : 0;
    int64_t firstChunkLength = options.TransferOptions.InitialChunkSize;
    if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
    {
      firstChunkLength = std::min(firstChunkLength, options.Range.GetValue().Length.GetValue());
    }

    DownloadBlobOptions firstChunkOptions;
    firstChunkOptions.Range = options.Range;
    if (firstChunkOptions.Range.HasValue())
    {
      firstChunkOptions.Range.GetValue().Length = firstChunkLength;
    }

    Storage::Details::FileWriter fileWriter(fileName);

    auto firstChunk = Download(firstChunkOptions, context);
    const Azure::Core::ETag eTag = firstChunk->Details.ETag;

    const int64_t blobSize = firstChunk->BlobSize;
    int64_t blobRangeSize;
    if (firstChunkOptions.Range.HasValue())
    {
      blobRangeSize = blobSize - firstChunkOffset;
      if (options.Range.HasValue() && options.Range.GetValue().Length.HasValue())
      {
        blobRangeSize = std::min(blobRangeSize, options.Range.GetValue().Length.GetValue());
      }
    }
    else
    {
      blobRangeSize = blobSize;
    }
    firstChunkLength = std::min(firstChunkLength, blobRangeSize);

    auto bodyStreamToFile = [](Azure::Core::Http::BodyStream& stream,
                               Storage::Details::FileWriter& fileWriter,
                               int64_t offset,
                               int64_t length,
                               const Azure::Core::Context& context) {
      constexpr std::size_t bufferSize = 4 * 1024 * 1024;
      std::vector<uint8_t> buffer(bufferSize);
      while (length > 0)
      {
        int64_t readSize = std::min(static_cast<int64_t>(bufferSize), length);
        int64_t bytesRead
            = Azure::Core::Http::BodyStream::ReadToCount(context, stream, buffer.data(), readSize);
        if (bytesRead != readSize)
        {
          throw Azure::Core::RequestFailedException("error when reading body stream");
        }
        fileWriter.Write(buffer.data(), bytesRead, offset);
        length -= bytesRead;
        offset += bytesRead;
      }
    };

    bodyStreamToFile(*(firstChunk->BodyStream), fileWriter, 0, firstChunkLength, context);
    firstChunk->BodyStream.reset();

    auto returnTypeConverter = [](Azure::Core::Response<Models::DownloadBlobResult>& response) {
      Models::DownloadBlobToResult ret;
      ret.BlobType = std::move(response->BlobType);
      ret.ContentRange = std::move(response->ContentRange);
      ret.BlobSize = response->BlobSize;
      ret.TransactionalContentHash = std::move(response->TransactionalContentHash);
      ret.Details = std::move(response->Details);
      return Azure::Core::Response<Models::DownloadBlobToResult>(
          std::move(ret), response.ExtractRawResponse());
    };
    auto ret = returnTypeConverter(firstChunk);

    // Keep downloading the remaining in parallel
    auto downloadChunkFunc
        = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
            DownloadBlobOptions chunkOptions;
            chunkOptions.Range = Core::Http::Range();
            chunkOptions.Range.GetValue().Offset = offset;
            chunkOptions.Range.GetValue().Length = length;
            if (!chunkOptions.AccessConditions.IfMatch.HasValue())
            {
              chunkOptions.AccessConditions.IfMatch = eTag;
            }
            auto chunk = Download(chunkOptions, context);
            bodyStreamToFile(
                *(chunk->BodyStream),
                fileWriter,
                offset - firstChunkOffset,
                chunkOptions.Range.GetValue().Length.GetValue(),
                context);

            if (chunkId == numChunks - 1)
            {
              ret = returnTypeConverter(chunk);
              ret->TransactionalContentHash.Reset();
            }
          };

    int64_t remainingOffset = firstChunkOffset + firstChunkLength;
    int64_t remainingSize = blobRangeSize - firstChunkLength;

    Storage::Details::ConcurrentTransfer(
        remainingOffset,
        remainingSize,
        options.TransferOptions.ChunkSize,
        options.TransferOptions.Concurrency,
        downloadChunkFunc);
    ret->ContentRange.Offset = firstChunkOffset;
    ret->ContentRange.Length = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::GetBlobPropertiesResult> BlobClient::GetProperties(
      const GetBlobPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::GetBlobPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    auto response = Details::BlobRestClient::Blob::GetProperties(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
    if (response->Tier.HasValue() && !response->IsAccessTierInferred.HasValue())
    {
      response->IsAccessTierInferred = false;
    }
    if (response->VersionId.HasValue() && !response->IsCurrentVersion.HasValue())
    {
      response->IsCurrentVersion = false;
    }
    if (response->CopyStatus.HasValue() && !response->IsIncrementalCopy.HasValue())
    {
      response->IsIncrementalCopy = false;
    }
    if (response->BlobType == Models::BlobType::AppendBlob && !response->IsSealed.HasValue())
    {
      response->IsSealed = false;
    }
    return response;
  }

  Azure::Core::Response<Models::SetBlobHttpHeadersResult> BlobClient::SetHttpHeaders(
      Models::BlobHttpHeaders httpHeaders,
      const SetBlobHttpHeadersOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::SetBlobHttpHeadersOptions protocolLayerOptions;
    protocolLayerOptions.HttpHeaders = std::move(httpHeaders);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::Blob::SetHttpHeaders(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobMetadataResult> BlobClient::SetMetadata(
      Metadata metadata,
      const SetBlobMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::SetBlobMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::Blob::SetMetadata(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobAccessTierResult> BlobClient::SetAccessTier(
      Models::AccessTier tier,
      const SetBlobAccessTierOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::SetBlobAccessTierOptions protocolLayerOptions;
    protocolLayerOptions.Tier = tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    return Details::BlobRestClient::Blob::SetAccessTier(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::StartCopyBlobResult> BlobClient::StartCopyFromUri(
      const std::string& sourceUri,
      const StartCopyBlobFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::StartCopyBlobFromUriOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.ShouldSealDestination = options.ShouldSealDestination;
    protocolLayerOptions.SourceIfTags = options.SourceAccessConditions.TagConditions;

    auto response = Details::BlobRestClient::Blob::StartCopyFromUri(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
    Models::StartCopyBlobResult res;
    res.RequestId = std::move(response->RequestId);
    res.ETag = std::move(response->ETag);
    res.LastModified = std::move(response->LastModified);
    res.CopyId = std::move(response->CopyId);
    res.CopyStatus = std::move(response->CopyStatus);
    res.VersionId = std::move(response->VersionId);
    res.m_blobClient = std::make_shared<BlobClient>(*this);
    return Azure::Core::Response<Models::StartCopyBlobResult>(
        std::move(res), response.ExtractRawResponse());
  }

  Azure::Core::Response<Models::AbortCopyBlobFromUriResult> BlobClient::AbortCopyFromUri(
      const std::string& copyId,
      const AbortCopyBlobFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::AbortCopyBlobFromUriOptions protocolLayerOptions;
    protocolLayerOptions.CopyId = copyId;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::Blob::AbortCopyFromUri(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateBlobSnapshotResult> BlobClient::CreateSnapshot(
      const CreateBlobSnapshotOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::CreateBlobSnapshotOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::Blob::CreateSnapshot(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteBlobResult> BlobClient::Delete(
      const DeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::DeleteBlobOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::Blob::Delete(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteBlobResult> BlobClient::DeleteIfExists(
      const DeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::NotFound
          && (e.ErrorCode == "BlobNotFound" || e.ErrorCode == "ContainerNotFound"))
      {
        Models::DeleteBlobResult ret;
        ret.RequestId = e.RequestId;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteBlobResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::UndeleteBlobResult> BlobClient::Undelete(
      const UndeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    Details::BlobRestClient::Blob::UndeleteBlobOptions protocolLayerOptions;
    return Details::BlobRestClient::Blob::Undelete(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobTagsResult> BlobClient::SetTags(
      std::map<std::string, std::string> tags,
      const SetBlobTagsOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::SetBlobTagsOptions protocolLayerOptions;
    protocolLayerOptions.Tags = std::move(tags);
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::Blob::SetTags(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetBlobTagsResult> BlobClient::GetTags(
      const GetBlobTagsOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::Blob::GetBlobTagsOptions protocolLayerOptions;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::Blob::GetTags(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

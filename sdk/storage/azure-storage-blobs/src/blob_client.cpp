// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/concurrent_transfer.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/file_io.hpp"
#include "azure/storage/common/reliable_stream.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient BlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::Details::ParseConnectionString(connectionString);
    auto blobUri = std::move(parsedConnectionString.BlobServiceUri);
    blobUri.AppendPath(Storage::Details::UrlEncodePath(containerName));
    blobUri.AppendPath(Storage::Details::UrlEncodePath(blobName));

    if (parsedConnectionString.KeyCredential)
    {
      return BlobClient(blobUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobClient(blobUri.GetAbsoluteUrl(), options);
    }
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Storage::Details::c_StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(const std::string& blobUri, const BlobClientOptions& options)
      : m_blobUrl(blobUri), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlockBlobClient BlobClient::AsBlockBlobClient() const { return BlockBlobClient(*this); }

  AppendBlobClient BlobClient::AsAppendBlobClient() const { return AppendBlobClient(*this); }

  PageBlobClient BlobClient::AsPageBlobClient() const { return PageBlobClient(*this); }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::c_HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::c_HttpQuerySnapshot,
          Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlobClient BlobClient::WithVersionId(const std::string& versionId) const
  {
    BlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::c_HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::c_HttpQueryVersionId,
          Storage::Details::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Core::Response<Models::DownloadBlobResult> BlobClient::Download(
      const DownloadBlobOptions& options) const
  {
    Details::BlobRestClient::Blob::DownloadBlobOptions protocolLayerOptions;
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
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }

    auto downloadResponse = Details::BlobRestClient::Blob::Download(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);

    {
      // In case network failure during reading the body
      std::string eTag = downloadResponse->ETag;

      auto retryFunction
          = [this, options, eTag](
                const Azure::Core::Context& context,
                const HttpGetterInfo& retryInfo) -> std::unique_ptr<Azure::Core::Http::BodyStream> {
        unused(context);

        DownloadBlobOptions newOptions = options;
        newOptions.Offset
            = (options.Offset.HasValue() ? options.Offset.GetValue() : 0) + retryInfo.Offset;
        newOptions.Length = options.Length;
        if (newOptions.Length.HasValue())
        {
          newOptions.Length = options.Length.GetValue() - retryInfo.Offset;
        }
        if (!newOptions.AccessConditions.IfMatch.HasValue())
        {
          newOptions.AccessConditions.IfMatch = eTag;
        }
        return std::move(Download(newOptions)->BodyStream);
      };

      ReliableStreamOptions reliableStreamOptions;
      reliableStreamOptions.MaxRetryRequests = Storage::Details::c_reliableStreamRetryCount;
      downloadResponse->BodyStream = std::make_unique<ReliableStream>(
          std::move(downloadResponse->BodyStream), reliableStreamOptions, retryFunction);
    }
    return downloadResponse;
  }

  Azure::Core::Response<Models::DownloadBlobToResult> BlobClient::DownloadTo(
      uint8_t* buffer,
      std::size_t bufferSize,
      const DownloadBlobToOptions& options) const
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

    auto returnTypeConverter = [](Azure::Core::Response<Models::DownloadBlobResult>& response) {
      Models::DownloadBlobToResult ret;
      ret.ETag = response->ETag;
      ret.LastModified = std::move(response->LastModified);
      ret.HttpHeaders = std::move(response->HttpHeaders);
      ret.Metadata = std::move(response->Metadata);
      ret.BlobType = response->BlobType;
      ret.ServerEncrypted = response->ServerEncrypted;
      ret.EncryptionKeySha256 = std::move(response->EncryptionKeySha256);
      return Azure::Core::Response<Models::DownloadBlobToResult>(
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
            if (!chunkOptions.AccessConditions.IfMatch.HasValue())
            {
              chunkOptions.AccessConditions.IfMatch = firstChunk->ETag;
            }
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

    Storage::Details::ConcurrentTransfer(
        remainingOffset, remainingSize, chunkSize, options.Concurrency, downloadChunkFunc);
    ret->ContentLength = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::DownloadBlobToResult> BlobClient::DownloadTo(
      const std::string& fileName,
      const DownloadBlobToOptions& options) const
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

    Storage::Details::FileWriter fileWriter(fileName);

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
                               Storage::Details::FileWriter& fileWriter,
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

    auto returnTypeConverter = [](Azure::Core::Response<Models::DownloadBlobResult>& response) {
      Models::DownloadBlobToResult ret;
      ret.ETag = response->ETag;
      ret.LastModified = std::move(response->LastModified);
      ret.HttpHeaders = std::move(response->HttpHeaders);
      ret.Metadata = std::move(response->Metadata);
      ret.BlobType = response->BlobType;
      ret.ServerEncrypted = response->ServerEncrypted;
      ret.EncryptionKeySha256 = std::move(response->EncryptionKeySha256);
      return Azure::Core::Response<Models::DownloadBlobToResult>(
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
            if (!chunkOptions.AccessConditions.IfMatch.HasValue())
            {
              chunkOptions.AccessConditions.IfMatch = firstChunk->ETag;
            }
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

    Storage::Details::ConcurrentTransfer(
        remainingOffset, remainingSize, chunkSize, options.Concurrency, downloadChunkFunc);
    ret->ContentLength = blobRangeSize;
    return ret;
  }

  Azure::Core::Response<Models::GetBlobPropertiesResult> BlobClient::GetProperties(
      const GetBlobPropertiesOptions& options) const
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
    return Details::BlobRestClient::Blob::GetProperties(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobHttpHeadersResult> BlobClient::SetHttpHeaders(
      Models::BlobHttpHeaders httpHeaders,
      const SetBlobHttpHeadersOptions& options) const
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
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobMetadataResult> BlobClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetBlobMetadataOptions& options) const
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
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobAccessTierResult> BlobClient::SetAccessTier(
      Models::AccessTier Tier,
      const SetBlobAccessTierOptions& options) const
  {
    Details::BlobRestClient::Blob::SetBlobAccessTierOptions protocolLayerOptions;
    protocolLayerOptions.Tier = Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    return Details::BlobRestClient::Blob::SetAccessTier(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::StartCopyBlobFromUriResult> BlobClient::StartCopyFromUri(
      const std::string& sourceUri,
      const StartCopyBlobFromUriOptions& options) const
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
    protocolLayerOptions.SourceLeaseId = options.SourceConditions.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceConditions.IfNoneMatch;
    protocolLayerOptions.ShouldSealDestination = options.ShouldSealDestination;
    protocolLayerOptions.SourceIfTags = options.SourceConditions.TagConditions;
    return Details::BlobRestClient::Blob::StartCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::AbortCopyBlobFromUriResult> BlobClient::AbortCopyFromUri(
      const std::string& copyId,
      const AbortCopyBlobFromUriOptions& options) const
  {
    Details::BlobRestClient::Blob::AbortCopyBlobFromUriOptions protocolLayerOptions;
    protocolLayerOptions.CopyId = copyId;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::Blob::AbortCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateBlobSnapshotResult> BlobClient::CreateSnapshot(
      const CreateBlobSnapshotOptions& options) const
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
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteBlobResult> BlobClient::Delete(
      const DeleteBlobOptions& options) const
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
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::UndeleteBlobResult> BlobClient::Undelete(
      const UndeleteBlobOptions& options) const
  {
    Details::BlobRestClient::Blob::UndeleteBlobOptions protocolLayerOptions;
    return Details::BlobRestClient::Blob::Undelete(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::AcquireBlobLeaseResult> BlobClient::AcquireLease(
      const std::string& proposedLeaseId,
      int32_t duration,
      const AcquireBlobLeaseOptions& options) const
  {
    Details::BlobRestClient::Blob::AcquireBlobLeaseOptions protocolLayerOptions;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.LeaseDuration = duration;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::AcquireLease(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::RenewBlobLeaseResult> BlobClient::RenewLease(
      const std::string& leaseId,
      const RenewBlobLeaseOptions& options) const
  {
    Details::BlobRestClient::Blob::RenewBlobLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::RenewLease(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ReleaseBlobLeaseResult> BlobClient::ReleaseLease(
      const std::string& leaseId,
      const ReleaseBlobLeaseOptions& options) const
  {
    Details::BlobRestClient::Blob::ReleaseBlobLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::ReleaseLease(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ChangeBlobLeaseResult> BlobClient::ChangeLease(
      const std::string& leaseId,
      const std::string& proposedLeaseId,
      const ChangeBlobLeaseOptions& options) const
  {
    Details::BlobRestClient::Blob::ChangeBlobLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::ChangeLease(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::BreakBlobLeaseResult> BlobClient::BreakLease(
      const BreakBlobLeaseOptions& options) const
  {
    Details::BlobRestClient::Blob::BreakBlobLeaseOptions protocolLayerOptions;
    protocolLayerOptions.BreakPeriod = options.BreakPeriod;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::BreakLease(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobTagsResult> BlobClient::SetTags(
      std::map<std::string, std::string> tags,
      const SetBlobTagsOptions& options) const
  {
    Details::BlobRestClient::Blob::SetBlobTagsOptions protocolLayerOptions;
    protocolLayerOptions.Tags = std::move(tags);
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::SetTags(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetBlobTagsResult> BlobClient::GetTags(
      const GetBlobTagsOptions& options) const
  {
    Details::BlobRestClient::Blob::GetBlobTagsOptions protocolLayerOptions;
    protocolLayerOptions.IfTags = options.TagConditions;
    return Details::BlobRestClient::Blob::GetTags(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

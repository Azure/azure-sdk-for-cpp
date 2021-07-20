// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/block_blob_client.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <azure/core/io/body_stream.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/concurrent_transfer.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  BlockBlobClient BlockBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    BlockBlobClient newClient(BlobClient::CreateFromConnectionString(
        connectionString, blobContainerName, blobName, options));
    return newClient;
  }

  BlockBlobClient::BlockBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  BlockBlobClient::BlockBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  BlockBlobClient::BlockBlobClient(const std::string& blobUrl, const BlobClientOptions& options)
      : BlobClient(blobUrl, options)
  {
  }

  BlockBlobClient::BlockBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  BlockBlobClient BlockBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlockBlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(_internal::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          _internal::HttpQuerySnapshot, _internal::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlockBlobClient BlockBlobClient::WithVersionId(const std::string& versionId) const
  {
    BlockBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(_internal::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          _internal::HttpQueryVersionId, _internal::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Response<Models::UploadBlockBlobResult> BlockBlobClient::Upload(
      Azure::Core::IO::BodyStream& content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::UploadBlockBlobOptions protocolLayerOptions;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tags = options.Tags;
    protocolLayerOptions.AccessTier = options.AccessTier;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::BlockBlob::Upload(
        *m_pipeline, m_blobUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const uint8_t* buffer,
      size_t bufferSize,
      const UploadBlockBlobFromOptions& options,
      const Azure::Core::Context& context) const
  {
    constexpr int64_t DefaultStageBlockSize = 4 * 1024 * 1024ULL;
    constexpr int64_t MaxStageBlockSize = 4000 * 1024 * 1024ULL;
    constexpr int64_t MaxBlockNumber = 50000;
    constexpr int64_t BlockGrainSize = 1 * 1024 * 1024;

    if (static_cast<uint64_t>(options.TransferOptions.SingleUploadThreshold)
        > std::numeric_limits<size_t>::max())
    {
      throw Azure::Core::RequestFailedException("Single upload threshold is too big");
    }
    if (bufferSize <= static_cast<size_t>(options.TransferOptions.SingleUploadThreshold))
    {
      Azure::Core::IO::MemoryBodyStream contentStream(buffer, bufferSize);
      UploadBlockBlobOptions uploadBlockBlobOptions;
      uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
      uploadBlockBlobOptions.Metadata = options.Metadata;
      uploadBlockBlobOptions.Tags = options.Tags;
      uploadBlockBlobOptions.AccessTier = options.AccessTier;
      return Upload(contentStream, uploadBlockBlobOptions, context);
    }

    int64_t chunkSize;
    if (options.TransferOptions.ChunkSize.HasValue())
    {
      chunkSize = options.TransferOptions.ChunkSize.Value();
    }
    else
    {
      int64_t minChunkSize = (bufferSize + MaxBlockNumber - 1) / MaxBlockNumber;
      minChunkSize = (minChunkSize + BlockGrainSize - 1) / BlockGrainSize * BlockGrainSize;
      chunkSize = std::max(DefaultStageBlockSize, minChunkSize);
    }
    if (chunkSize > MaxStageBlockSize)
    {
      throw Azure::Core::RequestFailedException("Block size is too big.");
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Convert::Base64Encode(
          std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::IO::MemoryBodyStream contentStream(buffer + offset, static_cast<size_t>(length));
      StageBlockOptions chunkOptions;
      auto blockInfo = StageBlock(getBlockId(chunkId), contentStream, chunkOptions, context);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<size_t>(numChunks));
      }
    };

    _internal::ConcurrentTransfer(
        0, bufferSize, chunkSize, options.TransferOptions.Concurrency, uploadBlockFunc);

    for (size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tags = options.Tags;
    commitBlockListOptions.AccessTier = options.AccessTier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions, context);

    Models::UploadBlockBlobFromResult ret;
    ret.ETag = std::move(commitBlockListResponse.Value.ETag);
    ret.LastModified = std::move(commitBlockListResponse.Value.LastModified);
    ret.VersionId = std::move(commitBlockListResponse.Value.VersionId);
    ret.IsServerEncrypted = commitBlockListResponse.Value.IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(commitBlockListResponse.Value.EncryptionKeySha256);
    ret.EncryptionScope = std::move(commitBlockListResponse.Value.EncryptionScope);
    return Azure::Response<Models::UploadBlockBlobFromResult>(
        std::move(ret), std::move(commitBlockListResponse.RawResponse));
  }

  Azure::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const std::string& fileName,
      const UploadBlockBlobFromOptions& options,
      const Azure::Core::Context& context) const
  {
    constexpr int64_t DefaultStageBlockSize = 4 * 1024 * 1024ULL;
    constexpr int64_t MaxStageBlockSize = 4000 * 1024 * 1024ULL;
    constexpr int64_t MaxBlockNumber = 50000;
    constexpr int64_t BlockGrainSize = 1 * 1024 * 1024;

    {
      Azure::Core::IO::FileBodyStream contentStream(fileName);

      if (contentStream.Length() <= options.TransferOptions.SingleUploadThreshold)
      {
        UploadBlockBlobOptions uploadBlockBlobOptions;
        uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
        uploadBlockBlobOptions.Metadata = options.Metadata;
        uploadBlockBlobOptions.Tags = options.Tags;
        uploadBlockBlobOptions.AccessTier = options.AccessTier;
        return Upload(contentStream, uploadBlockBlobOptions, context);
      }
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Convert::Base64Encode(
          std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    _internal::FileReader fileReader(fileName);

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::IO::_internal::RandomAccessFileBodyStream contentStream(
          fileReader.GetHandle(), offset, length);
      StageBlockOptions chunkOptions;
      auto blockInfo = StageBlock(getBlockId(chunkId), contentStream, chunkOptions, context);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<size_t>(numChunks));
      }
    };

    int64_t chunkSize;
    if (options.TransferOptions.ChunkSize.HasValue())
    {
      chunkSize = options.TransferOptions.ChunkSize.Value();
    }
    else
    {
      int64_t minChunkSize = (fileReader.GetFileSize() + MaxBlockNumber - 1) / MaxBlockNumber;
      minChunkSize = (minChunkSize + BlockGrainSize - 1) / BlockGrainSize * BlockGrainSize;
      chunkSize = std::max(DefaultStageBlockSize, minChunkSize);
    }
    if (chunkSize > MaxStageBlockSize)
    {
      throw Azure::Core::RequestFailedException("Block size is too big.");
    }

    _internal::ConcurrentTransfer(
        0,
        fileReader.GetFileSize(),
        chunkSize,
        options.TransferOptions.Concurrency,
        uploadBlockFunc);

    for (size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tags = options.Tags;
    commitBlockListOptions.AccessTier = options.AccessTier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions, context);

    Models::UploadBlockBlobFromResult result;
    result.ETag = commitBlockListResponse.Value.ETag;
    result.LastModified = commitBlockListResponse.Value.LastModified;
    result.VersionId = commitBlockListResponse.Value.VersionId;
    result.IsServerEncrypted = commitBlockListResponse.Value.IsServerEncrypted;
    result.EncryptionKeySha256 = commitBlockListResponse.Value.EncryptionKeySha256;
    result.EncryptionScope = commitBlockListResponse.Value.EncryptionScope;
    return Azure::Response<Models::UploadBlockBlobFromResult>(
        std::move(result), std::move(commitBlockListResponse.RawResponse));
  }

  Azure::Response<Models::StageBlockResult> BlockBlobClient::StageBlock(
      const std::string& blockId,
      Azure::Core::IO::BodyStream& content,
      const StageBlockOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::BlockBlob::StageBlock(
        *m_pipeline, m_blobUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::StageBlockFromUriResult> BlockBlobClient::StageBlockFromUri(
      const std::string& blockId,
      const std::string& sourceUri,
      const StageBlockFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::StageBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.SourceRange = options.SourceRange;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::BlockBlob::StageBlockFromUri(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CommitBlockListResult> BlockBlobClient::CommitBlockList(
      const std::vector<std::string>& blockIds,
      const CommitBlockListOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::CommitBlockListOptions protocolLayerOptions;
    protocolLayerOptions.BlockList.reserve(blockIds.size());
    for (const auto& id : blockIds)
    {
      protocolLayerOptions.BlockList.emplace_back(std::make_pair(Models::BlockType::Latest, id));
    }
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tags = options.Tags;
    protocolLayerOptions.AccessTier = options.AccessTier;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::BlockBlob::CommitBlockList(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::GetBlockListResult> BlockBlobClient::GetBlockList(
      const GetBlockListOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::GetBlockListOptions protocolLayerOptions;
    protocolLayerOptions.ListType = options.ListType;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return _detail::BlobRestClient::BlockBlob::GetBlockList(
        *m_pipeline, m_blobUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
  }

}}} // namespace Azure::Storage::Blobs

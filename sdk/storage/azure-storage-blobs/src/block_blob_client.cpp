// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/block_blob_client.hpp"

#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/file_io.hpp>
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
      std::shared_ptr<Core::TokenCredential> credential,
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
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::HttpQuerySnapshot, Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlockBlobClient BlockBlobClient::WithVersionId(const std::string& versionId) const
  {
    BlockBlobClient newClient(*this);
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

  Azure::Core::Response<Models::UploadBlockBlobResult> BlockBlobClient::Upload(
      Azure::Core::Http::BodyStream* content,
      const UploadBlockBlobOptions& options) const
  {
    Details::BlobRestClient::BlockBlob::UploadBlockBlobOptions protocolLayerOptions;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
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
    return Details::BlobRestClient::BlockBlob::Upload(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadBlockBlobFromOptions& options) const
  {
    constexpr int64_t DefaultBlockSize = 8 * 1024 * 1024;
    constexpr int64_t MaximumNumberBlocks = 50000;
    constexpr int64_t GrainSize = 4 * 1024;

    int64_t chunkSize = DefaultBlockSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t minBlockSize = (bufferSize + MaximumNumberBlocks - 1) / MaximumNumberBlocks;
      chunkSize = std::max(chunkSize, minBlockSize);
      chunkSize = (chunkSize + GrainSize - 1) / GrainSize * GrainSize;
    }

    if (bufferSize <= static_cast<std::size_t>(chunkSize))
    {
      Azure::Core::Http::MemoryBodyStream contentStream(buffer, bufferSize);
      UploadBlockBlobOptions uploadBlockBlobOptions;
      uploadBlockBlobOptions.Context = options.Context;
      uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
      uploadBlockBlobOptions.Metadata = options.Metadata;
      uploadBlockBlobOptions.Tier = options.Tier;
      return Upload(&contentStream, uploadBlockBlobOptions);
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Base64Encode(std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::Http::MemoryBodyStream contentStream(buffer + offset, length);
      StageBlockOptions chunkOptions;
      chunkOptions.Context = options.Context;
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Storage::Details::ConcurrentTransfer(
        0, bufferSize, chunkSize, options.Concurrency, uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.Context = options.Context;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions);

    Models::UploadBlockBlobFromResult ret;
    ret.ETag = std::move(commitBlockListResponse->ETag);
    ret.LastModified = std::move(commitBlockListResponse->LastModified);
    ret.VersionId = std::move(commitBlockListResponse->VersionId);
    ret.IsServerEncrypted = commitBlockListResponse->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(commitBlockListResponse->EncryptionKeySha256);
    ret.EncryptionScope = std::move(commitBlockListResponse->EncryptionScope);
    return Azure::Core::Response<Models::UploadBlockBlobFromResult>(
        std::move(ret),
        std::make_unique<Azure::Core::Http::RawResponse>(
            std::move(commitBlockListResponse.GetRawResponse())));
  }

  Azure::Core::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const std::string& fileName,
      const UploadBlockBlobFromOptions& options) const
  {
    constexpr int64_t DefaultBlockSize = 8 * 1024 * 1024;
    constexpr int64_t MaximumNumberBlocks = 50000;
    constexpr int64_t GrainSize = 4 * 1024;

    Storage::Details::FileReader fileReader(fileName);

    int64_t chunkSize = DefaultBlockSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t minBlockSize
          = (fileReader.GetFileSize() + MaximumNumberBlocks - 1) / MaximumNumberBlocks;
      chunkSize = std::max(chunkSize, minBlockSize);
      chunkSize = (chunkSize + GrainSize - 1) / GrainSize * GrainSize;
    }

    if (fileReader.GetFileSize() <= chunkSize)
    {
      Azure::Core::Http::FileBodyStream contentStream(
          fileReader.GetHandle(), 0, fileReader.GetFileSize());
      UploadBlockBlobOptions uploadBlockBlobOptions;
      uploadBlockBlobOptions.Context = options.Context;
      uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
      uploadBlockBlobOptions.Metadata = options.Metadata;
      uploadBlockBlobOptions.Tier = options.Tier;
      return Upload(&contentStream, uploadBlockBlobOptions);
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Base64Encode(std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::Http::FileBodyStream contentStream(fileReader.GetHandle(), offset, length);
      StageBlockOptions chunkOptions;
      chunkOptions.Context = options.Context;
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Storage::Details::ConcurrentTransfer(
        0, fileReader.GetFileSize(), chunkSize, options.Concurrency, uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.Context = options.Context;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions);

    Models::UploadBlockBlobFromResult result;
    result.ETag = commitBlockListResponse->ETag;
    result.LastModified = commitBlockListResponse->LastModified;
    result.VersionId = commitBlockListResponse->VersionId;
    result.IsServerEncrypted = commitBlockListResponse->IsServerEncrypted;
    result.EncryptionKeySha256 = commitBlockListResponse->EncryptionKeySha256;
    result.EncryptionScope = commitBlockListResponse->EncryptionScope;
    return Azure::Core::Response<Models::UploadBlockBlobFromResult>(
        std::move(result),
        std::make_unique<Azure::Core::Http::RawResponse>(
            std::move(commitBlockListResponse.GetRawResponse())));
  }

  Azure::Core::Response<Models::StageBlockResult> BlockBlobClient::StageBlock(
      const std::string& blockId,
      Azure::Core::Http::BodyStream* content,
      const StageBlockOptions& options) const
  {
    Details::BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::BlockBlob::StageBlock(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<Models::StageBlockFromUriResult> BlockBlobClient::StageBlockFromUri(
      const std::string& blockId,
      const std::string& sourceUri,
      const StageBlockFromUriOptions& options) const
  {
    Details::BlobRestClient::BlockBlob::StageBlockFromUriOptions protocolLayerOptions;
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
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::BlockBlob::StageBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CommitBlockListResult> BlockBlobClient::CommitBlockList(
      const std::vector<std::string>& blockIds,
      const CommitBlockListOptions& options) const
  {
    Details::BlobRestClient::BlockBlob::CommitBlockListOptions protocolLayerOptions;
    protocolLayerOptions.BlockList.reserve(blockIds.size());
    for (const auto& id : blockIds)
    {
      protocolLayerOptions.BlockList.emplace_back(std::make_pair(Models::BlockType::Latest, id));
    }
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
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
    return Details::BlobRestClient::BlockBlob::CommitBlockList(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetBlockListResult> BlockBlobClient::GetBlockList(
      const GetBlockListOptions& options) const
  {
    Details::BlobRestClient::BlockBlob::GetBlockListOptions protocolLayerOptions;
    protocolLayerOptions.ListType = options.ListType;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::BlockBlob::GetBlockList(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

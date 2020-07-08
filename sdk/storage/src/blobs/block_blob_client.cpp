// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/block_blob_client.hpp"

#include "common/concurrent_transfer.hpp"
#include "common/crypt.hpp"
#include "common/file_io.hpp"
#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlockBlobClient BlockBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlockBlobClientOptions& options)
  {
    BlockBlobClient newClient(
        BlobClient::CreateFromConnectionString(connectionString, containerName, blobName, options));
    return newClient;
  }

  BlockBlobClient::BlockBlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlockBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  BlockBlobClient::BlockBlobClient(
      const std::string& blobUri,
      std::shared_ptr<TokenCredential> credential,
      const BlockBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  BlockBlobClient::BlockBlobClient(
      const std::string& blobUri,
      const BlockBlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
  }

  BlockBlobClient::BlockBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  BlockBlobClient BlockBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlockBlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQuery("snapshot");
    }
    else
    {
      newClient.m_blobUrl.AppendQuery("snapshot", snapshot);
    }
    return newClient;
  }

  BlobContentInfo BlockBlobClient::Upload(
      Azure::Core::Http::BodyStream& content,
      const UploadBlockBlobOptions& options) const
  {
    BlobRestClient::BlockBlob::UploadOptions protocolLayerOptions;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::BlockBlob::Upload(
        options.Context, *m_pipeline, m_blobUrl.ToString(), content, protocolLayerOptions);
  }

  BlobContentInfo BlockBlobClient::UploadFromBuffer(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadBlobOptions& options) const
  {
    constexpr int64_t c_defaultBlockSize = 8 * 1024 * 1024;
    constexpr int64_t c_maximumNumberBlocks = 50000;
    constexpr int64_t c_grainSize = 4 * 1024;

    int64_t chunkSize = c_defaultBlockSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t minBlockSize = (bufferSize + c_maximumNumberBlocks - 1) / c_maximumNumberBlocks;
      chunkSize = std::max(chunkSize, minBlockSize);
      chunkSize = (chunkSize + c_grainSize - 1) / c_grainSize * c_grainSize;
    }

    std::vector<std::pair<BlockType, std::string>> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t c_blockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(c_blockIdLength - blockId.length(), '0') + blockId;
      return Base64Encode(blockId);
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::Http::MemoryBodyStream contentStream(buffer + offset, length);
      StageBlockOptions chunkOptions;
      chunkOptions.Context = options.Context;
      auto blockInfo = StageBlock(getBlockId(chunkId), contentStream, chunkOptions);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Details::ConcurrentTransfer(0, bufferSize, chunkSize, options.Concurrency, uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i].first = BlockType::Uncommitted;
      blockIds[i].second = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.Context = options.Context;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions);
    commitBlockListResponse.ContentCRC64.Reset();
    commitBlockListResponse.ContentMD5.Reset();
    return commitBlockListResponse;
  }

  BlobContentInfo BlockBlobClient::UploadFromFile(
      const std::string& file,
      const UploadBlobOptions& options) const
  {
    constexpr int64_t c_defaultBlockSize = 8 * 1024 * 1024;
    constexpr int64_t c_maximumNumberBlocks = 50000;
    constexpr int64_t c_grainSize = 4 * 1024;

    Details::FileReader fileReader(file);

    int64_t chunkSize = c_defaultBlockSize;
    if (options.ChunkSize.HasValue())
    {
      chunkSize = options.ChunkSize.GetValue();
    }
    else
    {
      int64_t minBlockSize
          = (fileReader.GetFileSize() + c_maximumNumberBlocks - 1) / c_maximumNumberBlocks;
      chunkSize = std::max(chunkSize, minBlockSize);
      chunkSize = (chunkSize + c_grainSize - 1) / c_grainSize * c_grainSize;
    }

    std::vector<std::pair<BlockType, std::string>> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t c_blockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(c_blockIdLength - blockId.length(), '0') + blockId;
      return Base64Encode(blockId);
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::Core::Http::FileBodyStream contentStream(fileReader.GetHandle(), offset, length);
      StageBlockOptions chunkOptions;
      chunkOptions.Context = options.Context;
      auto blockInfo = StageBlock(getBlockId(chunkId), contentStream, chunkOptions);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Details::ConcurrentTransfer(
        0, fileReader.GetFileSize(), chunkSize, options.Concurrency, uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i].first = BlockType::Uncommitted;
      blockIds[i].second = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.Context = options.Context;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions);
    commitBlockListResponse.ContentCRC64.Reset();
    commitBlockListResponse.ContentMD5.Reset();
    return commitBlockListResponse;
  }

  BlockInfo BlockBlobClient::StageBlock(
      const std::string& blockId,
      Azure::Core::Http::BodyStream& content,
      const StageBlockOptions& options) const
  {
    BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    return BlobRestClient::BlockBlob::StageBlock(
        options.Context, *m_pipeline, m_blobUrl.ToString(), content, protocolLayerOptions);
  }

  BlockInfo BlockBlobClient::StageBlockFromUri(
      const std::string& blockId,
      const std::string& sourceUri,
      const StageBlockFromUriOptions& options) const
  {
    BlobRestClient::BlockBlob::StageBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.SourceUri = sourceUri;
    if (options.SourceOffset.HasValue() && options.SourceLength.HasValue())
    {
      protocolLayerOptions.SourceRange = std::make_pair(
          options.SourceOffset.GetValue(),
          options.SourceOffset.GetValue() + options.SourceLength.GetValue() - 1);
    }
    else if (options.SourceOffset.HasValue())
    {
      protocolLayerOptions.SourceRange = std::make_pair(
          options.SourceOffset.GetValue(),
          std::numeric_limits<
              std::remove_reference_t<decltype(options.SourceOffset.GetValue())>>::max());
    }
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceIfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceIfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceIfNoneMatch;
    return BlobRestClient::BlockBlob::StageBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobContentInfo BlockBlobClient::CommitBlockList(
      const std::vector<std::pair<BlockType, std::string>>& blockIds,
      const CommitBlockListOptions& options) const
  {
    BlobRestClient::BlockBlob::CommitBlockListOptions protocolLayerOptions;
    protocolLayerOptions.BlockList = blockIds;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::BlockBlob::CommitBlockList(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobBlockListInfo BlockBlobClient::GetBlockList(const GetBlockListOptions& options) const
  {
    BlobRestClient::BlockBlob::GetBlockListOptions protocolLayerOptions;
    protocolLayerOptions.ListType = options.ListType;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::BlockBlob::GetBlockList(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/block_blob_client.hpp"

#include "azure/storage/common/concurrent_transfer.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/file_io.hpp"
#include "azure/storage/common/storage_common.hpp"

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
      std::shared_ptr<Identity::ClientSecretCredential> credential,
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
      newClient.m_blobUrl.RemoveQueryParameter(Details::c_HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Details::c_HttpQuerySnapshot, Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlockBlobClient BlockBlobClient::WithVersionId(const std::string& versionId) const
  {
    BlockBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Details::c_HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Details::c_HttpQueryVersionId, Details::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Core::Response<UploadBlockBlobResult> BlockBlobClient::Upload(
      Azure::Core::Http::BodyStream* content,
      const UploadBlockBlobOptions& options) const
  {
    BlobRestClient::BlockBlob::UploadBlockBlobOptions protocolLayerOptions;
    protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentMd5;
    protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentCrc64;
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
    return BlobRestClient::BlockBlob::Upload(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadBlockBlobFromOptions& options) const
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
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions);
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

    UploadBlockBlobFromResult ret;
    ret.ETag = std::move(commitBlockListResponse->ETag);
    ret.LastModified = std::move(commitBlockListResponse->LastModified);
    ret.VersionId = std::move(commitBlockListResponse->VersionId);
    ret.ServerEncrypted = commitBlockListResponse->ServerEncrypted;
    ret.EncryptionKeySha256 = std::move(commitBlockListResponse->EncryptionKeySha256);
    ret.EncryptionScope = std::move(commitBlockListResponse->EncryptionScope);
    return Azure::Core::Response<UploadBlockBlobFromResult>(
        std::move(ret),
        std::make_unique<Azure::Core::Http::RawResponse>(
            std::move(commitBlockListResponse.GetRawResponse())));
  }

  Azure::Core::Response<UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const std::string& file,
      const UploadBlockBlobFromOptions& options) const
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
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions);
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

    UploadBlockBlobFromResult result;
    result.ETag = commitBlockListResponse->ETag;
    result.LastModified = commitBlockListResponse->LastModified;
    result.VersionId = commitBlockListResponse->VersionId;
    result.ServerEncrypted = commitBlockListResponse->ServerEncrypted;
    result.EncryptionKeySha256 = commitBlockListResponse->EncryptionKeySha256;
    result.EncryptionScope = commitBlockListResponse->EncryptionScope;
    return Azure::Core::Response<UploadBlockBlobFromResult>(
        std::move(result),
        std::make_unique<Azure::Core::Http::RawResponse>(
            std::move(commitBlockListResponse.GetRawResponse())));
  }

  Azure::Core::Response<StageBlockResult> BlockBlobClient::StageBlock(
      const std::string& blockId,
      Azure::Core::Http::BodyStream* content,
      const StageBlockOptions& options) const
  {
    BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentMd5;
    protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::BlockBlob::StageBlock(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<StageBlockFromUriResult> BlockBlobClient::StageBlockFromUri(
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
    protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentMd5;
    protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::BlockBlob::StageBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<CommitBlockListResult> BlockBlobClient::CommitBlockList(
      const std::vector<std::pair<BlockType, std::string>>& blockIds,
      const CommitBlockListOptions& options) const
  {
    BlobRestClient::BlockBlob::CommitBlockListOptions protocolLayerOptions;
    protocolLayerOptions.BlockList = blockIds;
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
    return BlobRestClient::BlockBlob::CommitBlockList(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetBlockListResult> BlockBlobClient::GetBlockList(
      const GetBlockListOptions& options) const
  {
    BlobRestClient::BlockBlob::GetBlockListOptions protocolLayerOptions;
    protocolLayerOptions.ListType = options.ListType;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return BlobRestClient::BlockBlob::GetBlockList(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

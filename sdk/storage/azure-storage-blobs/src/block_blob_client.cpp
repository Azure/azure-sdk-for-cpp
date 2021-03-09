// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/block_blob_client.hpp"

#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

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
      newClient.m_blobUrl.RemoveQueryParameter(Storage::_detail::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::_detail::HttpQuerySnapshot, Storage::_detail::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  BlockBlobClient BlockBlobClient::WithVersionId(const std::string& versionId) const
  {
    BlockBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::_detail::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::_detail::HttpQueryVersionId,
          Storage::_detail::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Response<Models::UploadBlockBlobResult> BlockBlobClient::Upload(
      Azure::IO::BodyStream* content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::UploadBlockBlobOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::BlockBlob::Upload(
        *m_pipeline, m_blobUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadBlockBlobFromOptions& options,
      const Azure::Core::Context& context) const
  {
    constexpr int64_t MaxStageBlockSize = 4000 * 1024 * 1024ULL;

    int64_t chunkSize = std::min(MaxStageBlockSize, options.TransferOptions.ChunkSize);

    if (bufferSize <= static_cast<std::size_t>(options.TransferOptions.SingleUploadThreshold))
    {
      Azure::IO::MemoryBodyStream contentStream(buffer, bufferSize);
      UploadBlockBlobOptions uploadBlockBlobOptions;
      uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
      uploadBlockBlobOptions.Metadata = options.Metadata;
      uploadBlockBlobOptions.Tier = options.Tier;
      return Upload(&contentStream, uploadBlockBlobOptions, context);
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Base64Encode(std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::IO::MemoryBodyStream contentStream(buffer + offset, length);
      StageBlockOptions chunkOptions;
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions, context);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Storage::_detail::ConcurrentTransfer(
        0, bufferSize, chunkSize, options.TransferOptions.Concurrency, uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions, context);

    Models::UploadBlockBlobFromResult ret;
    ret.ETag = std::move(commitBlockListResponse->ETag);
    ret.LastModified = std::move(commitBlockListResponse->LastModified);
    ret.VersionId = std::move(commitBlockListResponse->VersionId);
    ret.IsServerEncrypted = commitBlockListResponse->IsServerEncrypted;
    ret.EncryptionKeySha256 = std::move(commitBlockListResponse->EncryptionKeySha256);
    ret.EncryptionScope = std::move(commitBlockListResponse->EncryptionScope);
    return Azure::Response<Models::UploadBlockBlobFromResult>(
        std::move(ret), commitBlockListResponse.ExtractRawResponse());
  }

  Azure::Response<Models::UploadBlockBlobFromResult> BlockBlobClient::UploadFrom(
      const std::string& fileName,
      const UploadBlockBlobFromOptions& options,
      const Azure::Core::Context& context) const
  {
    constexpr int64_t MaxStageBlockSize = 4000 * 1024 * 1024ULL;

    Storage::_detail::FileReader fileReader(fileName);

    int64_t chunkSize = std::min(MaxStageBlockSize, options.TransferOptions.ChunkSize);

    if (fileReader.GetFileSize() <= options.TransferOptions.SingleUploadThreshold)
    {
      Azure::IO::FileBodyStream contentStream(fileReader.GetHandle(), 0, fileReader.GetFileSize());
      UploadBlockBlobOptions uploadBlockBlobOptions;
      uploadBlockBlobOptions.HttpHeaders = options.HttpHeaders;
      uploadBlockBlobOptions.Metadata = options.Metadata;
      uploadBlockBlobOptions.Tier = options.Tier;
      return Upload(&contentStream, uploadBlockBlobOptions, context);
    }

    std::vector<std::string> blockIds;
    auto getBlockId = [](int64_t id) {
      constexpr std::size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Base64Encode(std::vector<uint8_t>(blockId.begin(), blockId.end()));
    };

    auto uploadBlockFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      Azure::IO::FileBodyStream contentStream(fileReader.GetHandle(), offset, length);
      StageBlockOptions chunkOptions;
      auto blockInfo = StageBlock(getBlockId(chunkId), &contentStream, chunkOptions, context);
      if (chunkId == numChunks - 1)
      {
        blockIds.resize(static_cast<std::size_t>(numChunks));
      }
    };

    Storage::_detail::ConcurrentTransfer(
        0,
        fileReader.GetFileSize(),
        chunkSize,
        options.TransferOptions.Concurrency,
        uploadBlockFunc);

    for (std::size_t i = 0; i < blockIds.size(); ++i)
    {
      blockIds[i] = getBlockId(static_cast<int64_t>(i));
    }
    CommitBlockListOptions commitBlockListOptions;
    commitBlockListOptions.HttpHeaders = options.HttpHeaders;
    commitBlockListOptions.Metadata = options.Metadata;
    commitBlockListOptions.Tier = options.Tier;
    auto commitBlockListResponse = CommitBlockList(blockIds, commitBlockListOptions, context);

    Models::UploadBlockBlobFromResult result;
    result.ETag = commitBlockListResponse->ETag;
    result.LastModified = commitBlockListResponse->LastModified;
    result.VersionId = commitBlockListResponse->VersionId;
    result.IsServerEncrypted = commitBlockListResponse->IsServerEncrypted;
    result.EncryptionKeySha256 = commitBlockListResponse->EncryptionKeySha256;
    result.EncryptionScope = commitBlockListResponse->EncryptionScope;
    return Azure::Response<Models::UploadBlockBlobFromResult>(
        std::move(result), commitBlockListResponse.ExtractRawResponse());
  }

  Azure::Response<Models::StageBlockResult> BlockBlobClient::StageBlock(
      const std::string& blockId,
      Azure::IO::BodyStream* content,
      const StageBlockOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
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
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
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
        *m_pipeline, m_blobUrl, protocolLayerOptions, Storage::_detail::WithReplicaStatus(context));
  }

}}} // namespace Azure::Storage::Blobs

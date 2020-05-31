// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/block_blob_client.hpp"

#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlockBlobClient BlockBlobClient::FromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlockBlobClientOptions& options)
  {
    BlockBlobClient newClient(
        BlobClient::FromConnectionString(connectionString, containerName, blobName, options));
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

  BlockBlobClient::BlockBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient))
  {
  }

  BlockBlobClient BlockBlobClient::WithSnapshot(const std::string& snapshot)
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
      // TODO: We don't have BodyStream for now.
      std::vector<uint8_t> content,
      const UploadBlobOptions& options)
  {
    BlobRestClient::BlockBlob::UploadOptions protocolLayerOptions;
    protocolLayerOptions.BodyBuffer = &content;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.BlobType = BlobType::BlockBlob;
    protocolLayerOptions.Properties = options.Properties;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    return BlobRestClient::BlockBlob::Upload(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlockInfo BlockBlobClient::StageBlock(
      const std::string& blockId,
      // TODO: We don't have BodyStream for now.
      std::vector<uint8_t> content,
      const StageBlockOptions& options)
  {
    BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BodyBuffer = &content;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    return BlobRestClient::BlockBlob::StageBlock(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobContentInfo BlockBlobClient::CommitBlockList(
      const std::vector<std::pair<BlockType, std::string>>& blockIds,
      const CommitBlockListOptions& options)
  {
    BlobRestClient::BlockBlob::CommitBlockListOptions protocolLayerOptions;
    protocolLayerOptions.BlockList = blockIds;
    protocolLayerOptions.Properties = options.Properties;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    return BlobRestClient::BlockBlob::CommitBlockList(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobBlockListInfo BlockBlobClient::GetBlockList(const GetBlockListOptions& options)
  {
    BlobRestClient::BlockBlob::GetBlockListOptions protocolLayerOptions;
    protocolLayerOptions.ListType = options.ListType;
    return BlobRestClient::BlockBlob::GetBlockList(m_blobUrl.to_string(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

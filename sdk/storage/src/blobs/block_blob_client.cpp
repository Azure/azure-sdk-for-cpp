// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/block_blob_client.hpp"

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
      std::unique_ptr<Azure::Core::Http::BodyStream> content,
      const UploadBlobOptions& options) const
  {
    BlobRestClient::BlockBlob::UploadOptions protocolLayerOptions;
    protocolLayerOptions.BodyStream = std::move(content);
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.Properties = options.Properties;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::BlockBlob::Upload(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlockInfo BlockBlobClient::StageBlock(
      const std::string& blockId,
      std::unique_ptr<Azure::Core::Http::BodyStream> content,
      const StageBlockOptions& options) const
  {
    BlobRestClient::BlockBlob::StageBlockOptions protocolLayerOptions;
    protocolLayerOptions.BodyStream = std::move(content);
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    return BlobRestClient::BlockBlob::StageBlock(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlockInfo BlockBlobClient::StageBlockFromUri(
      const std::string& blockId,
      const std::string& sourceUri,
      const StageBlockFromUriOptions& options) const
  {
    BlobRestClient::BlockBlob::StageBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.BlockId = blockId;
    protocolLayerOptions.SourceUri = sourceUri;
    if (options.SourceOffset != std::numeric_limits<decltype(options.SourceOffset)>::max())
    {
      if (options.SourceLength == 0)
      {
        protocolLayerOptions.SourceRange = std::make_pair(
            options.SourceOffset, std::numeric_limits<decltype(options.SourceOffset)>::max());
      }
      else
      {
        protocolLayerOptions.SourceRange
            = std::make_pair(options.SourceOffset, options.SourceOffset + options.SourceLength - 1);
      }
    }
    else
    {
      protocolLayerOptions.SourceRange
          = std::make_pair(std::numeric_limits<uint64_t>::max(), uint64_t(0));
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
    protocolLayerOptions.Properties = options.Properties;
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

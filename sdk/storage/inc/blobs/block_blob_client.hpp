// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>

#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "blob_client_options.hpp"
#include "internal/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  class BlockBlobClient : public BlobClient {
  public:
    // connection string
    static BlockBlobClient FromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    // shared key auth
    explicit BlockBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    // token auth
    explicit BlockBlobClient(
        const std::string& blobUri,
        std::shared_ptr<TokenCredential> credential,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlockBlobClient(
        const std::string& blobUri,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    BlockBlobClient WithSnapshot(const std::string& snapshot);

    BlobContentInfo Upload(
        // TODO: We don't have BodyStream for now.
        std::vector<uint8_t> content,
        const UploadBlobOptions& options = UploadBlobOptions());

    BlockInfo StageBlock(
        const std::string& blockId,
        // TODO: We don't have BodyStream for now.
        std::vector<uint8_t> content,
        const StageBlockOptions& options = StageBlockOptions());

    BlobContentInfo CommitBlockList(
        const std::vector<std::pair<BlockType, std::string>>& blockIds,
        const CommitBlockListOptions& options = CommitBlockListOptions());

    BlobBlockListInfo GetBlockList(const GetBlockListOptions& options = GetBlockListOptions());

  private:
    explicit BlockBlobClient(BlobClient blobClient);
  };

}}} // namespace Azure::Storage::Blobs

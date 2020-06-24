// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <map>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  class BlockBlobClient : public BlobClient {
  public:
    // connection string
    static BlockBlobClient CreateFromConnectionString(
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

    BlockBlobClient WithSnapshot(const std::string& snapshot) const;

    BlobContentInfo Upload(
        std::unique_ptr<Azure::Core::Http::BodyStream> content,
        const UploadBlobOptions& options = UploadBlobOptions()) const;

    BlockInfo StageBlock(
        const std::string& blockId,
        std::unique_ptr<Azure::Core::Http::BodyStream> content,
        const StageBlockOptions& options = StageBlockOptions()) const;

    BlockInfo StageBlockFromUri(
        const std::string& blockId,
        const std::string& sourceUri,
        const StageBlockFromUriOptions& options = StageBlockFromUriOptions()) const;

    BlobContentInfo CommitBlockList(
        const std::vector<std::pair<BlockType, std::string>>& blockIds,
        const CommitBlockListOptions& options = CommitBlockListOptions()) const;

    BlobBlockListInfo GetBlockList(
        const GetBlockListOptions& options = GetBlockListOptions()) const;

  private:
    explicit BlockBlobClient(BlobClient blobClient);
    friend class BlobClient;
  };

}}} // namespace Azure::Storage::Blobs

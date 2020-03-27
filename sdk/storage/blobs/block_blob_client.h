// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class BlockBlobClient : public BlobClient
{
public:
    // connection string
    BlockBlobClient(const std::string& connectionString, std::string containerName, std::string blobName, BlobClientOptions options = BlobClientOptions(() : BlobClient(connectionString, std::move(containerName), std::move(blobName), std::move(options)) {}

    // shared key auth
    BlockBlobClient(Azure::Core::Http::Uri uri, Azure::Storage::Common::StorageSharedKeyCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // token auth
    BlockBlobClient(Azure::Core::Http::Uri uri, Azure::Core::TokenCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // anonymous/SAS/customized pipeline auth
    BlockBlobClient(Azure::Core::Http::Uri uri, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(options)) {}

    Azure::Core::Http::Response<BlockInfo> StageBlock(const std::string& blockId, std::istream& content, const StageBlockOptions& options = StageBlockOptions());
    Azure::Core::Http::Response<BlockInfo> StageBlock(const std::string& blockId, char* buffer, const StageBlockOptions& options = StageBlockOptions());
    Azure::Core::Http::Response<BlockInfo> StageBlock(const std::string& blockId, const Azure::Core::Http::Uri& url, const StageBlockOptions& options = StageBlockOptions());

    Azure::Core::Http::Response<BlobContentInfo> CommitBlockList(const std::vector<std::string>& blockIds, const CommitBlockListOptions& options = CommitBlockListOptions());

    Azure::Core::Http::Response<BlockList> GetBlockList(const GetBlockListOptions& options = GetBlockListOptions());

    // parallel upload
    BlobContentInfo UploadFrom(std::istream& content, const UploadBlockBlobOptions& options = UploadBlockBlobOptions());
    BlobContentInfo UploadFrom(const std::string& path, const UploadBlockBlobOptions& options = UploadBlockBlobOptions());
    BlobContentInfo UploadFrom(const char* buffer, uint64_t size, const UploadBlockBlobOptions& options = UploadBlockBlobOptions());
};

}}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class AppendBlobClient : public BlobClient
{
public:
    // connection string
    AppendBlobClient(const std::string& connectionString, std::string containerName, std::string blobName, BlobClientOptions options = BlobClientOptions()) : BlobClient(connectionString, std::move(containerName), std::move(blobName), std::move(options)) {}

    // shared key auth
    AppendBlobClient(Azure::Core::Http::Uri uri, Azure::Storage::Common::StorageSharedKeyCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // token auth
    AppendBlobClient(Azure::Core::Http::Uri uri, Azure::Core::TokenCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // anonymous/SAS/customized pipeline auth
    AppendBlobClient(Azure::Core::Http::Uri uri, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(options)) {}

    Azure::Core::Http::Response<BlobContentInfo> Create(const CreateAppendBlobOptions& options = CreateAppendBlobOptions());

    Azure::Core::Http::Response<AppendBlockInfo> AppendBlock(std::istream& content, const AppendBlockOptions& options = AppendBlockOptions());
    Azure::Core::Http::Response<AppendBlockInfo> AppendBlock(const char* buffer, uint64_t size, const AppendBlockOptions& options = AppendBlockOptions());
    Azure::Core::Http::Response<AppendBlockInfo> AppendBlock(const Azure::Core::Http::Uri& url, const AppendBlockOptions& options = AppendBlockOptions());

    // parallel upload
    BlobContentInfo UploadFrom(std::istream& content, const UploadAppendBlobOptions& options = UploadAppendBlobOptions());
    BlobContentInfo UploadFrom(const std::string& path, const UploadAppendBlobOptions& options = UploadAppendBlobOptions());
    BlobContentInfo UploadFrom(const char* buffer, uint64_t size, const UploadAppendBlobOptions& options = UploadAppendBlobOptions());

};

}}

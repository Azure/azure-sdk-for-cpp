// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class PageBlobClient : public BlobClient
{
public:
    // connection string
    PageBlobClient(const std::string& connectionString, std::string containerName, std::string blobName, BlobClientOptions options = BlobClientOptions()) : BlobClient(connectionString, std::move(containerName), std::move(blobName), std::move(options)) {}

    // shared key auth
    PageBlobClient(Azure::Core::Http::Uri uri, Azure::Storage::Common::StorageSharedKeyCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // token auth
    PageBlobClient(Azure::Core::Http::Uri uri, Azure::Core::TokenCredential credential, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(credential), std::move(options)) {}

    // anonymous/SAS/customized pipeline auth
    PageBlobClient(Azure::Core::Http::Uri uri, BlobClientOptions options = BlobClientOptions()) : BlobClient(std::move(uri), std::move(options)) {}

    Azure::Core::Http::Response<BlobContentInfo> Create(uint64_t size, const CreatePageBlobOptions& option = CreatePageBlobOptions());

    Azure::Core::Http::Response<PageInfo> UploadPages(std::istream& content, uint64_t offset, const UploadPagesOptions& options = UploadPagesOptions());
    Azure::Core::Http::Response<PageInfo> UploadPages(const char* buffer, uint64_t offset, uint64_t size, const UploadPagesOptions& options = UploadPagesOptions());
    Azure::Core::Http::Response<PageInfo> UploadPages(const Azure::Core::Http::Uri& url, uint64_t sourceOffset, uint64_t offset, uint64_t size, const UploadPagesOptions& options = UploadPagesOptions());

    Azure::Core::Http::Response<PageInfo> ClearPages(uint64_t offset, uint64_t size, const ClearPagesOptions& options = ClearPagesOptions());

    Azure::Core::Http::Response<PageRangesInfo> GetPageRanges(uint64_t offset, uint64_t size, const GetPageRangesOptions& options = GetPageRangesOptions());

    Azure::Core::Http::Response<PageRangesInfo> GetPageRangesDiff(uint64_t offset, uint64_t size, const std::string& previousSnapshot, const GetPageRangesDiffOptions& options = GetPageRangesDiffOptions());

    Azure::Core::Http::Response<BlobContentInfo> Resize(uint64_t size, const ResizePageBlobOptions& options = ResizePageBlobOptions());

    Azure::Core::Http::Response<BlobContentInfo> UpdateSequenceNumber(SequenceNumberAction action, const UpdatePageSequenceNumberOptions& options = UpdatePageSequenceNumberOptions());

};

}}

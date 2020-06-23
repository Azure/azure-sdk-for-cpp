// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  struct PageRange
  {
    uint64_t Offset;
    uint64_t Length;
  };

  struct PageRangesInfo
  {
    std::string RequestId;
    std::string Date;
    std::string Version;
    std::string ClientRequestId;
    std::string ETag;
    std::string LastModified;
    uint64_t BlobContentLength = 0;
    std::vector<PageRange> PageRanges;
    std::vector<PageRange> ClearRanges;
  };

  class PageBlobClient : public BlobClient {
  public:
    // connection string
    static PageBlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    // shared key auth
    explicit PageBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    // token auth
    explicit PageBlobClient(
        const std::string& blobUri,
        std::shared_ptr<TokenCredential> credential,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit PageBlobClient(
        const std::string& blobUri,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    PageBlobClient WithSnapshot(const std::string& snapshot) const;

    BlobContentInfo Create(
        uint64_t blobContentLength,
        const CreatePageBlobOptions& options = CreatePageBlobOptions());

    PageInfo UploadPages(
        Azure::Core::Http::BodyStream* content,
        uint64_t offset,
        const UploadPagesOptions& options = UploadPagesOptions());

    PageInfo UploadPagesFromUri(
        std::string sourceUri,
        uint64_t sourceOffset,
        uint64_t sourceLength,
        uint64_t destinationoffset,
        const UploadPagesFromUriOptions& options = UploadPagesFromUriOptions());

    PageInfo ClearPages(
        uint64_t offset,
        uint64_t length,
        const ClearPagesOptions& options = ClearPagesOptions());

    PageBlobInfo Resize(
        uint64_t blobContentLength,
        const ResizePageBlobOptions& options = ResizePageBlobOptions());

    PageRangesInfo GetPageRanges(const GetPageRangesOptions& options = GetPageRangesOptions());

    BlobCopyInfo StartCopyIncremental(
        const std::string& sourceUri,
        const IncrementalCopyPageBlobOptions& options = IncrementalCopyPageBlobOptions());

  private:
    explicit PageBlobClient(BlobClient blobClient);
    friend class BlobClient;
  };

}}} // namespace Azure::Storage::Blobs

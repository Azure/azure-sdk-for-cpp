// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <map>
#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  class BlockBlobClient;
  class AppendBlobClient;
  class PageBlobClient;

  class BlobClient {
  public:
    // connection string
    static BlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const BlobClientOptions& options = BlobClientOptions());

    // shared key auth
    explicit BlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    // token auth
    explicit BlobClient(
        const std::string& blobUri,
        std::shared_ptr<TokenCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlobClient(
        const std::string& blobUri,
        const BlobClientOptions& options = BlobClientOptions());

    BlockBlobClient GetBlockBlobClient() const;

    AppendBlobClient GetAppendBlobClient() const;

    PageBlobClient GetPageBlobClient() const;

    std::string GetUri() const { return m_blobUrl.ToString(); }

    BlobClient WithSnapshot(const std::string& snapshot) const;

    BlobProperties GetProperties(
        const GetBlobPropertiesOptions& options = GetBlobPropertiesOptions()) const;

    BlobInfo SetHttpHeaders(
        const SetBlobHttpHeadersOptions& options = SetBlobHttpHeadersOptions()) const;

    BlobInfo SetMetadata(
        std::map<std::string, std::string> metadata,
        const SetBlobMetadataOptions& options = SetBlobMetadataOptions()) const;

    BasicResponse SetAccessTier(
        AccessTier Tier,
        const SetAccessTierOptions& options = SetAccessTierOptions()) const;

    BlobCopyInfo StartCopyFromUri(
        const std::string& sourceUri,
        const StartCopyFromUriOptions& options = StartCopyFromUriOptions()) const;

    BasicResponse AbortCopyFromUri(
        const std::string& copyId,
        const AbortCopyFromUriOptions& options = AbortCopyFromUriOptions()) const;

    FlattenedDownloadProperties Download(
        const DownloadBlobOptions& options = DownloadBlobOptions()) const;

    BlobSnapshotInfo CreateSnapshot(
        const CreateSnapshotOptions& options = CreateSnapshotOptions()) const;

    BasicResponse Delete(const DeleteBlobOptions& options = DeleteBlobOptions()) const;

    BasicResponse Undelete(const UndeleteBlobOptions& options = UndeleteBlobOptions()) const;

  protected:
    UrlBuilder m_blobUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

  private:
    BlobClient() = default;
    friend class BlobContainerClient;
  };
}}} // namespace Azure::Storage::Blobs

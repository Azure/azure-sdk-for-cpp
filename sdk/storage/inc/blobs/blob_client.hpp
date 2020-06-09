// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_client_options.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <map>
#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

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

    BlobClient WithSnapshot(const std::string& snapshot);

    BlobProperties GetProperties(
        const GetBlobPropertiesOptions& options = GetBlobPropertiesOptions());

    BlobInfo SetHttpHeaders(const SetBlobHttpHeadersOptions& options = SetBlobHttpHeadersOptions());

    BlobInfo SetMetadata(
        std::map<std::string, std::string> metadata,
        const SetBlobMetadataOptions& options = SetBlobMetadataOptions());

    FlattenedDownloadProperties Download(
        const DownloadBlobOptions& options = DownloadBlobOptions());

    BasicResponse Delete(const DeleteBlobOptions& options = DeleteBlobOptions());

  protected:
    UrlBuilder m_blobUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}} // namespace Azure::Storage::Blobs

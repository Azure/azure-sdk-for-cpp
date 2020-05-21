// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <map>

#include "common/storage_credential.hpp"
#include "internal/blob_client_options.hpp"
#include "internal/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  class BlobClient {
  public:
    // connection string
    static BlobClient FromConnectionString(
        const std::string& connectionString,
        std::string containerName,
        std::string blobName,
        BlobClientOptions options = BlobClientOptions());

    // shared key auth
    explicit BlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        BlobClientOptions options = BlobClientOptions());

    // token auth
    explicit BlobClient(
        const std::string& blobUri,
        std::shared_ptr<TokenCredential> credential,
        BlobClientOptions options = BlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlobClient(
        const std::string& blobUri,
        BlobClientOptions options = BlobClientOptions());

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
    std::string m_blobUrl;
  };
}}} // namespace Azure::Storage::Blobs

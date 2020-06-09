// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_container_client_options.hpp"
#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <map>
#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  class BlobContainerClient {
  public:
    // connection string
    static BlobContainerClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    // shared key auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    // token auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<TokenCredential> credential,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    BlobClient GetBlobClient(const std::string& blobName);

    BlobContainerInfo Create(
        const CreateBlobContainerOptions& options = CreateBlobContainerOptions());

    BasicResponse Delete(const DeleteBlobContainerOptions& options = DeleteBlobContainerOptions());

    BlobContainerProperties GetProperties(
        const GetBlobContainerPropertiesOptions& options = GetBlobContainerPropertiesOptions());

    BlobContainerInfo SetMetadata(
        std::map<std::string, std::string> metadata,
        SetBlobContainerMetadataOptions options = SetBlobContainerMetadataOptions());

    BlobsFlatSegment ListBlobs(const ListBlobsOptions& options = ListBlobsOptions());

  private:
    UrlBuilder m_ContainerUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };

}}} // namespace Azure::Storage::Blobs

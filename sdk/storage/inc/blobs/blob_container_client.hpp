// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <map>

#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "internal/blob_container_client_options.hpp"
#include "internal/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  class BlobContainerClient {
  public:
    // connection string
    static BlobContainerClient FromConnectionString(
        const std::string& connectionString,
        std::string containerName,
        BlobContainerClientOptions options = BlobContainerClientOptions());

    // shared key auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<SharedKeyCredential> credential,
        BlobContainerClientOptions options = BlobContainerClientOptions());

    // token auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<TokenCredential> credential,
        BlobContainerClientOptions options = BlobContainerClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlobContainerClient(
        const std::string& containerUri,
        BlobContainerClientOptions options = BlobContainerClientOptions());

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
    std::string m_ContainerUri;
  };

}}} // namespace Azure::Storage::Blobs

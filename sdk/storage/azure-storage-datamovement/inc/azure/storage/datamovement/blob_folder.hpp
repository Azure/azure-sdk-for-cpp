// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace DataMovement {

  class BlobFolder final {
  public:
    static BlobFolder CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& blobContainerName,
        const std::string& blobName,
        const Blobs::BlobClientOptions& options = Blobs::BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const Blobs::BlobClientOptions& options = Blobs::BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const Blobs::BlobClientOptions& options = Blobs::BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        const Blobs::BlobClientOptions& options = Blobs::BlobClientOptions());

    BlobFolder GetBlobFolder(const std::string& folderName);

    Blobs::BlobClient GetBlobClient(const std::string& blobName);
  };

  BlobFolder GetBlobFolderFromBlobContainer(
      const Blobs::BlobContainerClient& blobContainerClient,
      const std::string& folderName);

}}} // namespace Azure::Storage::DataMovement

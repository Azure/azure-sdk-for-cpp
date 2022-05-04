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

    std::string GetUrl() const { return m_blobUrl.GetAbsoluteUrl(); }
    
    BlobFolder GetBlobFolder(const std::string& folderName) const;

    Blobs::BlobClient GetBlobClient(const std::string& blobName) const;

  private:
    Azure::Core::Url m_blobUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    Azure::Nullable<Blobs::EncryptionKey> m_customerProvidedKey;
    Azure::Nullable<std::string> m_encryptionScope;
  };

}}} // namespace Azure::Storage::DataMovement

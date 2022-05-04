// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  class BlobFolder final {
  public:
    static BlobFolder CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& blobContainerName,
        const std::string& blobName,
        const BlobClientOptions& options = BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    explicit BlobFolder(
        const std::string& blobUrl,
        const BlobClientOptions& options = BlobClientOptions());

    std::string GetUrl() const { return m_blobUrl.GetAbsoluteUrl(); }

    BlobFolder GetBlobFolder(const std::string& folderName) const;

    BlobClient GetBlobClient(const std::string& blobName) const;

  private:
    Core::Url m_blobUrl;
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Nullable<EncryptionKey> m_customerProvidedKey;
    Nullable<std::string> m_encryptionScope;
  };

}}} // namespace Azure::Storage::Blobs

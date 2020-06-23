// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  class AppendBlobClient : public BlobClient {
  public:
    // connection string
    static AppendBlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    // shared key auth
    explicit AppendBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    // token auth
    explicit AppendBlobClient(
        const std::string& blobUri,
        std::shared_ptr<TokenCredential> credential,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit AppendBlobClient(
        const std::string& blobUri,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    AppendBlobClient WithSnapshot(const std::string& snapshot) const;

    BlobContentInfo Create(const CreateAppendBlobOptions& options = CreateAppendBlobOptions());

    BlobAppendInfo AppendBlock(
        Azure::Core::Http::BodyStream* content,
        const AppendBlockOptions& options = AppendBlockOptions());

    BlobAppendInfo AppendBlockFromUri(
        const std::string& sourceUri,
        const AppendBlockFromUriOptions& options = AppendBlockFromUriOptions()) const;

  private:
    explicit AppendBlobClient(BlobClient blobClient);
    friend class BlobClient;
  };

}}} // namespace Azure::Storage::Blobs

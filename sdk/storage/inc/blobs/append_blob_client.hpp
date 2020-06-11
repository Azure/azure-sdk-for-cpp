// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_client_options.hpp"
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
        // TODO: We don't have BodyStream for now.
        std::vector<uint8_t> content,
        const AppendBlockOptions& options = AppendBlockOptions());

  private:
    explicit AppendBlobClient(BlobClient blobClient);
  };

}}} // namespace Azure::Storage::Blobs

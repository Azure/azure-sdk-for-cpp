// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_container_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  class BlobServiceClient {
  public:
    // connection string
    static BlobServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    // shared key auth
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    // token auth
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<TokenCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    // anonymous/SAS/customized pipeline auth
    explicit BlobServiceClient(
        const std::string& serviceUri,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    BlobContainerClient GetBlobContainerClient(const std::string& containerName) const;

    std::string GetUri() const { return m_serviceUrl.ToString(); }

    ListContainersSegment ListBlobContainersSegment(
        const ListBlobContainersOptions& options = ListBlobContainersOptions()) const;

    UserDelegationKey GetUserDelegationKey(
        const std::string& StartsOn,
        const std::string& expiresOn,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions()) const;

  protected:
    UrlBuilder m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}} // namespace Azure::Storage::Blobs

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "http/pipeline.hpp"
#include "protocol/share_rest_client.hpp"
#include "response.hpp"
#include "share_options.hpp"
#include "share_responses.hpp"
#include "shares/share_service_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static ShareClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param shareUri The URI of the file share this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the share's primary uri endpoint.
     *
     * @return The share's primary uri endpoint.
     */
    std::string GetUri() const { return m_shareUri.ToString(); }

    /**
     * @brief Creates the file share.
     * @param options Optional parameters to create this file share.
     * @return Azure::Core::Response<ShareInfo> The information containing the version and modified
     * time of a share.
     */
    Azure::Core::Response<ShareInfo> Create(
        const CreateShareOptions& options = CreateShareOptions()) const;

    /**
     * @brief Deletes the file share.
     * @param options Optional parameters to delete this file share.
     * @return Azure::Core::Response<ShareDeleteInfo> Currently empty and reserved for future usage.
     */
    Azure::Core::Response<ShareDeleteInfo> Delete(
        const DeleteShareOptions& options = DeleteShareOptions()) const;

  private:
    UriBuilder m_shareUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareClient(
        UriBuilder shareUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareUri(std::move(shareUri)), m_pipeline(std::move(pipeline))
    {
    }
    friend class ServiceClient;
  };
}}}} // namespace Azure::Storage::Files::Shares

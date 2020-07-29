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

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ServiceClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param options Optional parameters used to initialize the client.
     * @return ServiceClient
     */
    static ServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const ServiceClientOptions& options = ServiceClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param serviceUri The service URI this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const ServiceClientOptions& options = ServiceClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param serviceUri The service URI this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const ServiceClientOptions& options = ServiceClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param serviceUri The service URI this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ServiceClient(
        const std::string& serviceUri,
        const ServiceClientOptions& options = ServiceClientOptions());

    /**
     * @brief Gets the file share service's primary uri endpoint.
     *
     * @return The file share service's primary uri endpoint.
     */
    std::string GetUri() const { return m_serviceUri.ToString(); }

    /**
     * @brief List the shares from the service.
     * @param options Optional parameters to list the shares.
     * @return Azure::Core::Response<ListSharesResult>
     */
    Azure::Core::Response<ListSharesSegmentResult> ListSharesSegment(
        const ListSharesOptions& options = ListSharesOptions()) const;

  private:
    UriBuilder m_serviceUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}}} // namespace Azure::Storage::Files::Shares

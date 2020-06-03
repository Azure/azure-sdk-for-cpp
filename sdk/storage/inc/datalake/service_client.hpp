// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace DataLake {

  class FileSystemClient;

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
        std::shared_ptr<TokenCredential> credential,
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
     * @brief Create a FileSystemClient from current ServiceClient
     * @param fileSystemName The name of the file system.
     * @return FileSystemClient
     */
    FileSystemClient GetFileSystemClient(const std::string& fileSystemName) const;

    /**
     * @brief List the file systems from the service.
     * @param options Optional parameters to list the file systems.
     * @return ServiceListFileSystemsResponse
     */
    ServiceListFileSystemsResponse ListFileSystems(
        const ListFileSystemsOptions& options = ListFileSystemsOptions()) const;

  private:
    UrlBuilder m_dfsUri;
    UrlBuilder m_blobUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}} // namespace Azure::Storage::DataLake

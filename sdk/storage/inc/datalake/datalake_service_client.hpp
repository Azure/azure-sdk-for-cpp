// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blobs/blob_service_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "datalake_options.hpp"
#include "datalake_responses.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"
#include "response.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

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
     * @brief Create a FileSystemClient from current ServiceClient
     * @param fileSystemName The name of the file system.
     * @return FileSystemClient
     */
    FileSystemClient GetFileSystemClient(const std::string& fileSystemName) const;

    /**
     * @brief Gets the datalake service's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The datalake service's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobServiceClient.GetUri(); }

    /**
     * @brief Gets the datalake service's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The datalake service's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.ToString(); }

    /**
     * @brief List the file systems from the service.
     * @param options Optional parameters to list the file systems.
     * @return Azure::Core::Response<ListFileSystemsResult>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<ListFileSystemsResult> ListFileSystems(
        const ListFileSystemsOptions& options = ListFileSystemsOptions()) const;

    /**
     * @brief Retrieves a key that can be used to delegate Active Directory authorization to
     * shared access signatures.
     *
     * @param startsOn Start time for the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param expiresOn Expiration of the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param options Optional parameters to execute
     * this function.
     * @return Azure::Core::Response<UserDelegationKey>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<UserDelegationKey> GetUserDelegationKey(
        const std::string& startsOn,
        const std::string& expiresOn,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions()) const
    {
      return m_blobServiceClient.GetUserDelegationKey(startsOn, expiresOn, options);
    }

  private:
    UriBuilder m_dfsUri;
    Blobs::BlobServiceClient m_blobServiceClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}}} // namespace Azure::Storage::Files::DataLake

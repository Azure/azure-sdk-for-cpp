// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/blobs/blob_service_client.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakeFileSystemClient;

  class DataLakeServiceClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param options Optional parameters used to initialize the client.
     * @return DataLakeServiceClient
     */
    static DataLakeServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param serviceUrl The service URL this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param serviceUrl The service URL this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param serviceUrl The service URL this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeServiceClient(
        const std::string& serviceUrl,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Create a FileSystemClient from current DataLakeServiceClient
     * @param fileSystemName The name of the file system.
     * @return FileSystemClient
     */
    DataLakeFileSystemClient GetFileSystemClient(const std::string& fileSystemName) const;

    /**
     * @brief Gets the datalake service's primary url endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The datalake service's primary url endpoint.
     */
    std::string GetUrl() const { return m_blobServiceClient.GetUrl(); }

    /**
     * @brief List the file systems from the service.
     * @param options Optional parameters to list the file systems.
     * @return Azure::Core::Response<Models::ListFileSystemsSinglePageResult> containing the
     * listed result of file systems and continuation token for unfinished list result.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::ListFileSystemsSinglePageResult> ListFileSystemsSinglePage(
        const ListFileSystemsSinglePageOptions& options = ListFileSystemsSinglePageOptions()) const;

    /**
     * @brief Retrieves a key that can be used to delegate Active Directory authorization to
     * shared access signatures.
     *
     * @param expiresOn Expiration of the key's validity. The time should be specified in UTC, and
     * will be truncated to second.
     * @param options Optional parameters to execute
     * this function.
     * @return Azure::Core::Response<Models::GetUserDelegationKeyResult> containing the user
     * delegation key related information.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetUserDelegationKeyResult> GetUserDelegationKey(
        const Azure::Core::DateTime& expiresOn,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions()) const
    {
      return m_blobServiceClient.GetUserDelegationKey(expiresOn, options);
    }

  private:
    Azure::Core::Http::Url m_serviceUrl;
    Blobs::BlobServiceClient m_blobServiceClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/blobs/blob_container_client.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/datalake_service_client.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakePathClient;
  class DataLakeFileClient;
  class DataLakeDirectoryClient;

  class DataLakeFileSystemClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param options Optional parameters used to initialize the client.
     * @return FileSystemClient
     */
    static DataLakeFileSystemClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUri,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Create a DataLakePathClient from current DataLakeFileSystemClient
     * @param path Path of the resource within the file system.
     * @return DataLakePathClient
     */
    DataLakePathClient GetPathClient(const std::string& path) const;

    /**
     * @brief Create a DataLakeFileClient from current DataLakeFileSystemClient
     * @param path Path of the file within the file system.
     * @return DataLakeFileClient
     */
    DataLakeFileClient GetFileClient(const std::string& path) const;

    /**
     * @brief Create a DataLakeDirectoryClient from current DataLakeFileSystemClient
     * @param path Path of the directory within the file system.
     * @return DataLakeDirectoryClient
     */
    DataLakeDirectoryClient GetDirectoryClient(const std::string& path) const;

    /**
     * @brief Gets the filesystem's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The filesystem's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobContainerClient.GetUrl(); }

    /**
     * @brief Gets the filesystem's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The filesystem's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.GetAbsoluteUrl(); }

    /**
     * @brief Creates the file system.
     * @param options Optional parameters to create this file system.
     * @return Azure::Core::Response<Models::CreateDataLakeFileSystemResult> containing the
     * information of create a file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileSystemResult> Create(
        const CreateDataLakeFileSystemOptions& options = CreateDataLakeFileSystemOptions()) const;

    /**
     * @brief Creates the file system if it does not exists.
     * @param options Optional parameters to create this file system.
     * @return Azure::Core::Response<Models::CreateDataLakeFileSystemResult> containing the
     * information of create a file system. Only valid when successfully created the file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileSystemResult> CreateIfNotExists(
        const CreateDataLakeFileSystemOptions& options = CreateDataLakeFileSystemOptions()) const;

    /**
     * @brief Deletes the file system.
     * @param options Optional parameters to delete this file system.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> containing the
     * information returned when deleting file systems.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> Delete(
        const DeleteDataLakeFileSystemOptions& options = DeleteDataLakeFileSystemOptions()) const;

    /**
     * @brief Deletes the file system if it exists.
     * @param options Optional parameters to delete this file system.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> containing the
     * information returned when deleting file systems. Only valid when successfully deleted the
     * file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> DeleteIfExists(
        const DeleteDataLakeFileSystemOptions& options = DeleteDataLakeFileSystemOptions()) const;

    /**
     * @brief Sets the metadata of file system.
     * @param metadata User-defined metadata to be stored with the filesystem. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set the metadata to this file system.
     * @return Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult> containing the
     * information returned when setting the metadata onto the file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetDataLakeFileSystemMetadataOptions& options
        = SetDataLakeFileSystemMetadataOptions()) const;

    /**
     * @brief Gets the properties of file system.
     * @param options Optional parameters to get the metadata of this file system.
     * @return Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult> containing the
     * information when getting the file system's properties.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult> GetProperties(
        const GetDataLakeFileSystemPropertiesOptions& options
        = GetDataLakeFileSystemPropertiesOptions()) const;

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, only paths at the root of the
     *                  filesystem are listed. If "directory" is specified, the list will only
     *                  include paths that share the same root.
     * @param options Optional parameters to list the paths in file system.
     * @return Azure::Core::Response<Models::ListPathsSinglePageResult> containing the
     * results when listing the paths under a file system.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::ListPathsSinglePageResult> ListPathsSinglePage(
        bool recursive,
        const ListPathsSinglePageOptions& options = ListPathsSinglePageOptions()) const;

  private:
    Azure::Core::Http::Url m_dfsUri;
    Blobs::BlobContainerClient m_blobContainerClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit DataLakeFileSystemClient(
        Azure::Core::Http::Url dfsUri,
        Blobs::BlobContainerClient blobContainerClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_dfsUri(std::move(dfsUri)), m_blobContainerClient(std::move(blobContainerClient)),
          m_pipeline(std::move(pipeline))
    {
    }
    friend class DataLakeServiceClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

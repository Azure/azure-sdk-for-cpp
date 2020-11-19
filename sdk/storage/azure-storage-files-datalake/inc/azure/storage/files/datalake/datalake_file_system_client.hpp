// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/datalake_service_client.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class PathClient;
  class FileClient;
  class DirectoryClient;

  class FileSystemClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param options Optional parameters used to initialize the client.
     * @return FileSystemClient
     */
    static FileSystemClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The client secret credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Create a PathClient from current FileSystemClient
     * @param path Path of the resource within the file system.
     * @return PathClient
     */
    PathClient GetPathClient(const std::string& path) const;

    /**
     * @brief Create a FileClient from current FileSystemClient
     * @param path Path of the file within the file system.
     * @return FileClient
     */
    FileClient GetFileClient(const std::string& path) const;

    /**
     * @brief Create a DirectoryClient from current FileSystemClient
     * @param path Path of the directory within the file system.
     * @return DirectoryClient
     */
    DirectoryClient GetDirectoryClient(const std::string& path) const;

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
     * @return Azure::Core::Response<Models::CreateFileSystemResult> containing the information of
     * create a file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::CreateFileSystemResult> Create(
        const CreateFileSystemOptions& options = CreateFileSystemOptions()) const;

    /**
     * @brief Deletes the file system.
     * @param options Optional parameters to delete this file system.
     * @return Azure::Core::Response<Models::DeleteFileSystemResult> containing the information
     * returned when deleting file systems.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DeleteFileSystemResult> Delete(
        const DeleteFileSystemOptions& options = DeleteFileSystemOptions()) const;

    /**
     * @brief Sets the metadata of file system.
     * @param metadata User-defined metadata to be stored with the filesystem. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set the metadata to this file system.
     * @return Azure::Core::Response<Models::SetFileSystemMetadataResult> containing the information
     * returned when setting the metadata onto the file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetFileSystemMetadataResult> SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const SetFileSystemMetadataOptions& options = SetFileSystemMetadataOptions()) const;

    /**
     * @brief Gets the properties of file system.
     * @param options Optional parameters to get the metadata of this file system.
     * @return Azure::Core::Response<Models::GetFileSystemPropertiesResult> containing the
     * information when getting the file system's properties.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetFileSystemPropertiesResult> GetProperties(
        const GetFileSystemPropertiesOptions& options = GetFileSystemPropertiesOptions()) const;

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, only paths at the root of the
     *                  filesystem are listed. If "directory" is specified, the list will only
     *                  include paths that share the same root.
     * @param options Optional parameters to list the paths in file system.
     * @return Azure::Core::Response<Models::ListPathsResult> containing the results when listing
     * the paths under a file system.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::ListPathsResult> ListPaths(
        bool recursive,
        const ListPathsOptions& options = ListPathsOptions()) const;

  private:
    Azure::Core::Http::Url m_dfsUri;
    Blobs::BlobContainerClient m_blobContainerClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit FileSystemClient(
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

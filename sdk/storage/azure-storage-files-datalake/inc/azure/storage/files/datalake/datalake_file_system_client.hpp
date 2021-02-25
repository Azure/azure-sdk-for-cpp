// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/credentials.hpp>
#include <azure/core/internal/http/pipeline.hpp>
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
     * @param fileSystemUrl The URL of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileSystemUrl The URL of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileSystemUrl The URL of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileSystemClient(
        const std::string& fileSystemUrl,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Create a DataLakeFileClient from current DataLakeFileSystemClient
     * @param fileName Name of the file within the file system.
     * @return DataLakeFileClient
     */
    DataLakeFileClient GetFileClient(const std::string& fileName) const;

    /**
     * @brief Create a DataLakeDirectoryClient from current DataLakeFileSystemClient
     * @param directoryName Name of the directory within the file system.
     * @return DataLakeDirectoryClient
     */
    DataLakeDirectoryClient GetDirectoryClient(const std::string& directoryName) const;

    /**
     * @brief Gets the filesystem's primary url endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The filesystem's primary url endpoint.
     */
    std::string GetUrl() const { return m_blobContainerClient.GetUrl(); }

    /**
     * @brief Creates the file system.
     * @param options Optional parameters to create this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateDataLakeFileSystemResult> containing the
     * information of create a file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileSystemResult> Create(
        const CreateDataLakeFileSystemOptions& options = CreateDataLakeFileSystemOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates the file system if it does not exists.
     * @param options Optional parameters to create this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateDataLakeFileSystemResult> containing the
     * information of create a file system. Only valid when successfully created the file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileSystemResult> CreateIfNotExists(
        const CreateDataLakeFileSystemOptions& options = CreateDataLakeFileSystemOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file system.
     * @param options Optional parameters to delete this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> containing the
     * information returned when deleting file systems.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> Delete(
        const DeleteDataLakeFileSystemOptions& options = DeleteDataLakeFileSystemOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file system if it exists.
     * @param options Optional parameters to delete this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> containing the
     * information returned when deleting file systems. Only valid when successfully deleted the
     * file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> DeleteIfExists(
        const DeleteDataLakeFileSystemOptions& options = DeleteDataLakeFileSystemOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata of file system.
     * @param metadata User-defined metadata to be stored with the filesystem. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set the metadata to this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult> containing the
     * information returned when setting the metadata onto the file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetDataLakeFileSystemMetadataOptions& options
        = SetDataLakeFileSystemMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of file system.
     * @param options Optional parameters to get the metadata of this file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult> containing the
     * information when getting the file system's properties.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult> GetProperties(
        const GetDataLakeFileSystemPropertiesOptions& options
        = GetDataLakeFileSystemPropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, only paths at the root of the
     *                  filesystem are listed.
     * @param options Optional parameters to list the paths in file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ListPathsSinglePageResult> containing the
     * results when listing the paths under a file system.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::ListPathsSinglePageResult> ListPathsSinglePage(
        bool recursive,
        const ListPathsSinglePageOptions& options = ListPathsSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the permissions for this file system. The permissions indicate whether
     * file system data may be accessed publicly.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A GetDataLakeFileSystemAccessPolicyResult describing the container's access policy.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetDataLakeFileSystemAccessPolicyResult> GetAccessPolicy(
        const GetDataLakeFileSystemAccessPolicyOptions& options
        = GetDataLakeFileSystemAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the permissions for the specified file system. The permissions indicate
     * whether file system's data may be accessed publicly.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A SetDataLakeFileSystemAccessPolicyResult describing the updated file system.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetDataLakeFileSystemAccessPolicyResult> SetAccessPolicy(
        const SetDataLakeFileSystemAccessPolicyOptions& options
        = SetDataLakeFileSystemAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param fileName The file that gets renamed.
     * @param destinationFilePath The path of the file the source file is renaming to.
     * @param options Optional parameters to rename a file
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<DataLakeFileClient> The client targets the renamed file.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DataLakeFileClient> RenameFile(
        const std::string& fileName,
        const std::string& destinationFilePath,
        const RenameDataLakeFileOptions& options = RenameDataLakeFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param directoryName The directory that gets renamed.
     * @param destinationDirectoryPath The destinationPath the source directory is renaming to.
     * @param options Optional parameters to rename a directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<DataLakeDirectoryClient> The client targets the renamed
     * directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DataLakeDirectoryClient> RenameDirectory(
        const std::string& directoryName,
        const std::string& destinationDirectoryPath,
        const RenameDataLakeDirectoryOptions& options = RenameDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Internal::Http::Url m_fileSystemUrl;
    Blobs::BlobContainerClient m_blobContainerClient;
    std::shared_ptr<Azure::Core::Internal::Http::HttpPipeline> m_pipeline;

    explicit DataLakeFileSystemClient(
        Azure::Core::Internal::Http::Url fileSystemUrl,
        Blobs::BlobContainerClient blobContainerClient,
        std::shared_ptr<Azure::Core::Internal::Http::HttpPipeline> pipeline)
        : m_fileSystemUrl(std::move(fileSystemUrl)),
          m_blobContainerClient(std::move(blobContainerClient)), m_pipeline(std::move(pipeline))
    {
    }
    friend class DataLakeLeaseClient;
    friend class DataLakeServiceClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

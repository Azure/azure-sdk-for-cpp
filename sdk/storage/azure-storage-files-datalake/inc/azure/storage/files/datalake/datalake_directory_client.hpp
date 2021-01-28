// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <azure/core/credentials.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakeDirectoryClient : public DataLakePathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param directoryName The name of a directory within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return DataLakeDirectoryClient
     */
    static DataLakeDirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& directoryName,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUri,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUri,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUri,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Create a FileClient from current DataLakeDirectoryClient
     * @param fileName Name of the file under the directory.
     * @return FileClient
     */
    DataLakeFileClient GetFileClient(const std::string& fileName) const;

    /**
     * @brief Create a DataLakeDirectoryClient from current DataLakeDirectoryClient
     * @param subdirectoryName Name of the directory under the current directory.
     * @return DataLakeDirectoryClient
     */
    DataLakeDirectoryClient GetSubdirectoryClient(const std::string& subdirectoryName) const;

    /**
     * @brief Gets the directory's primary url endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The directory's primary url endpoint.
     */
    std::string GetUrl() const { return m_blobClient.GetUrl(); }

    /**
     * @brief Create a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the directory the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakeDirectoryResult> containing the
     * information of the created directory
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeDirectoryResult> Create(
        const CreateDataLakeDirectoryOptions& options = CreateDataLakeDirectoryOptions()) const
    {
      return DataLakePathClient::Create(Models::PathResourceType::Directory, options);
    }

    /**
     * @brief Create a directory. If it already exists, nothing will happen.
     * @param options Optional parameters to create the directory the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakeDirectoryResult> containing the
     * information of the created directory
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeDirectoryResult> CreateIfNotExists(
        const CreateDataLakeDirectoryOptions& options = CreateDataLakeDirectoryOptions()) const
    {
      return DataLakePathClient::CreateIfNotExists(Models::PathResourceType::Directory, options);
    }

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param fileName The file that gets renamed.
     * @param destinationFilePath The path of the file the source file is renaming to.
     * @param options Optional parameters to rename a file.
     * @return Azure::Core::Response<DataLakeFileClient> The client targets the renamed file.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DataLakeFileClient> RenameFile(
        const std::string& fileName,
        const std::string& destinationFilePath,
        const RenameDataLakeFileOptions& options = RenameDataLakeFileOptions()) const;

    /**
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param subdirectoryName The subdirectory that gets renamed.
     * @param destinationDirectoryPath The destinationPath the source subdirectory is renaming to.
     * @param options Optional parameters to rename a directory.
     * @return Azure::Core::Response<DataLakeDirectoryClient> The client targets the renamed
     * directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DataLakeDirectoryClient> RenameSubdirectory(
        const std::string& subdirectoryName,
        const std::string& destinationDirectoryPath,
        const RenameDataLakeSubdirectoryOptions& options
        = RenameDataLakeSubdirectoryOptions()) const;

    /**
     * @brief Deletes the empty directory. Errors if directory is not empty.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteIfEmpty(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const
    {
      return this->Delete(false, options);
    }

    /**
     * @brief Deletes the empty directory if it already exists. Errors if directory is not empty.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteIfEmptyIfExists(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const
    {
      return this->DeleteIfExists(false, options);
    }

    /**
     * @brief Deletes the directory and all its subfolders and files.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteRecursive(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const
    {
      return this->Delete(true, options);
    }

    /**
     * @brief Deletes the directory and all its subfolders and files if the directory exists.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteRecursiveIfExists(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const
    {
      return this->DeleteIfExists(true, options);
    }

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, the list will only
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
    explicit DataLakeDirectoryClient(
        Azure::Core::Http::Url dfsUrl,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : DataLakePathClient(std::move(dfsUrl), std::move(blobClient), pipeline)
    {
    }

    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> Delete(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const;

    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteIfExists(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const;

    // Hide path functions
    using DataLakePathClient::Delete;
    using DataLakePathClient::DeleteIfExists;
    friend class DataLakeFileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <azure/core/credentials.hpp>
#include <azure/core/internal/http/pipeline.hpp>
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
     * @param directoryUrl The URL of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param directoryUrl The URL of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param directoryUrl The URL of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeDirectoryClient(
        const std::string& directoryUrl,
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
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::CreateDataLakeDirectoryResult> containing the
     * information of the created directory
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::CreateDataLakeDirectoryResult> Create(
        const CreateDataLakeDirectoryOptions& options = CreateDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return DataLakePathClient::Create(Models::PathResourceType::Directory, options, context);
    }

    /**
     * @brief Create a directory. If it already exists, nothing will happen.
     * @param options Optional parameters to create the directory the path points to.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::CreateDataLakeDirectoryResult> containing the
     * information of the created directory
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::CreateDataLakeDirectoryResult> CreateIfNotExists(
        const CreateDataLakeDirectoryOptions& options = CreateDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return DataLakePathClient::CreateIfNotExists(
          Models::PathResourceType::Directory, options, context);
    }

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param fileName The file that gets renamed.
     * @param destinationFilePath The path of the file the source file is renaming to.
     * @param options Optional parameters to rename a file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<DataLakeFileClient> The client targets the renamed file.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<DataLakeFileClient> RenameFile(
        const std::string& fileName,
        const std::string& destinationFilePath,
        const RenameDataLakeFileOptions& options = RenameDataLakeFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param subdirectoryName The subdirectory that gets renamed.
     * @param destinationDirectoryPath The destinationPath the source subdirectory is renaming to.
     * @param options Optional parameters to rename a directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<DataLakeDirectoryClient> The client targets the renamed
     * directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<DataLakeDirectoryClient> RenameSubdirectory(
        const std::string& subdirectoryName,
        const std::string& destinationDirectoryPath,
        const RenameDataLakeSubdirectoryOptions& options = RenameDataLakeSubdirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the empty directory. Throws exception if directory is not empty.
     * @param options Optional parameters to delete the directory the path points to.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::DeleteDataLakeDirectoryResult> DeleteEmpty(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return this->Delete(false, options, context);
    }

    /**
     * @brief Deletes the empty directory if it already exists. Throws exception if directory is not
     * empty.
     * @param options Optional parameters to delete the directory the path points to.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::DeleteDataLakeDirectoryResult> DeleteEmptyIfExists(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return this->DeleteIfExists(false, options, context);
    }

    /**
     * @brief Deletes the directory and all its subdirectories and files.
     * @param options Optional parameters to delete the directory the path points to.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::DeleteDataLakeDirectoryResult> DeleteRecursive(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return this->Delete(true, options, context);
    }

    /**
     * @brief Deletes the directory and all its subdirectories and files if the directory exists.
     * @param options Optional parameters to delete the directory the path points to.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::DeleteDataLakeDirectoryResult> DeleteRecursiveIfExists(
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return this->DeleteIfExists(true, options, context);
    }

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, the list will only
     *                  include paths that share the same root.
     * @param options Optional parameters to list the paths in file system.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ListPathsSinglePageResult> containing the
     * results when listing the paths under a file system.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Response<Models::ListPathsSinglePageResult> ListPathsSinglePage(
        bool recursive,
        const ListPathsSinglePageOptions& options = ListPathsSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    explicit DataLakeDirectoryClient(
        Azure::Core::Http::Url directoryUrl,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
        : DataLakePathClient(std::move(directoryUrl), std::move(blobClient), pipeline)
    {
    }

    Azure::Response<Models::DeleteDataLakeDirectoryResult> Delete(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Response<Models::DeleteDataLakeDirectoryResult> DeleteIfExists(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    friend class DataLakeFileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

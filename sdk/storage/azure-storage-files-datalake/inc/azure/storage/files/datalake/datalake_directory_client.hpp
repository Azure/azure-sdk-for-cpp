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
     * @param directoryPath The path of a directory within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return DataLakeDirectoryClient
     */
    static DataLakeDirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& directoryPath,
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
     * @param path Path of the file under the directory.
     * @return FileClient
     */
    DataLakeFileClient GetFileClient(const std::string& path) const;

    /**
     * @brief Create a DataLakeDirectoryClient from current DataLakeDirectoryClient
     * @param path Path of the directory under the current directory.
     * @return DataLakeDirectoryClient
     */
    DataLakeDirectoryClient GetSubdirectoryClient(const std::string& path) const;

    /**
     * @brief Gets the directory's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The directory's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobClient.GetUrl(); }

    /**
     * @brief Gets the directory's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The directory's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.GetAbsoluteUrl(); }

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
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationDirectoryPath The destinationPath this current directory is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination
     * directory points to.
     * @return Azure::Core::Response<Models::RenameDataLakeDirectoryResult> containing the
     * information returned when renaming the directory.
     * @remark This operation will not change the URL this directory client points too, to use the
     *         new name, customer needs to initialize a new directory client with the new name/path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::RenameDataLakeDirectoryResult> Rename(
        const std::string& destinationDirectoryPath,
        const RenameDataLakeDirectoryOptions& options = RenameDataLakeDirectoryOptions()) const;

    /**
     * @brief Deletes the directory.
     * @param recursive If "true", all paths beneath the directory will be deleted. If "false" and
     *                  the directory is non-empty, an error occurs.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> Delete(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const;

    /**
     * @brief Deletes the directory if it already exists.
     * @param recursive If "true", all paths beneath the directory will be deleted. If "false" and
     *                  the directory is non-empty, an error occurs.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DeleteIfExists(
        bool recursive,
        const DeleteDataLakeDirectoryOptions& options = DeleteDataLakeDirectoryOptions()) const;

    /**
     * @brief Sets POSIX access control rights on files and directories under given directory
     * recursively.
     * @param mode Mode PathSetAccessControlRecursiveMode::Set sets POSIX access control rights on
     * files and directories, PathSetAccessControlRecursiveMode::Modify modifies one or more POSIX
     * access control rights  that pre-exist on files and directories,
     * PathSetAccessControlRecursiveMode::Remove removes one or more POSIX access control rights
     * that were present earlier on files and directories
     * @param acls Sets POSIX access control rights on files and directories. Each access control
     * entry (ACE) consists of a scope, a type, a user or group identifier, and permissions.
     * @param options Optional parameters to set an access control recursively to the resource the
     * directory points to.
     * @return Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveResult>
    SetAccessControlRecursive(
        Models::PathSetAccessControlRecursiveMode mode,
        std::vector<Models::Acl> acls,
        const SetDataLakeDirectoryAccessControlRecursiveOptions& options
        = SetDataLakeDirectoryAccessControlRecursiveOptions()) const;

  private:
    explicit DataLakeDirectoryClient(
        Azure::Core::Http::Url dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : DataLakePathClient(std::move(dfsUri), std::move(blobClient), pipeline)
    {
    }
    friend class DataLakeFileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

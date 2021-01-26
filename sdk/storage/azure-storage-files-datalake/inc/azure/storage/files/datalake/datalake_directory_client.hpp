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
     * @param acls Sets POSIX access control rights on files and directories. Each access control
     * entry (ACE) consists of a scope, a type, a user or group identifier, and permissions.
     * @param options Optional parameters to set an access control recursively to the resource the
     * directory points to.
     * @return
     * Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveSinglePageResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveSinglePageResult>
    SetAccessControlRecursiveSinglePage(
        std::vector<Models::Acl> acls,
        const SetDataLakeDirectoryAccessControlRecursiveSinglePageOptions& options
        = SetDataLakeDirectoryAccessControlRecursiveSinglePageOptions()) const
    {
      return SetAccessControlRecursiveSinglePageInternal(
          Models::PathSetAccessControlRecursiveMode::Set, acls, options);
    }

    /**
     * @brief Updates POSIX access control rights on files and directories under given directory
     * recursively.
     * @param acls Updates POSIX access control rights on files and directories. Each access control
     * entry (ACE) consists of a scope, a type, a user or group identifier, and permissions.
     * @param options Optional parameters to set an access control recursively to the resource the
     * directory points to.
     * @return
     * Azure::Core::Response<Models::UpdateDataLakeDirectoryAccessControlRecursiveSinglePageResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::UpdateDataLakeDirectoryAccessControlRecursiveSinglePageResult>
    UpdateAccessControlRecursiveSinglePage(
        std::vector<Models::Acl> acls,
        const UpdateDataLakeDirectoryAccessControlRecursiveSinglePageOptions& options
        = UpdateDataLakeDirectoryAccessControlRecursiveSinglePageOptions()) const
    {
      return SetAccessControlRecursiveSinglePageInternal(
          Models::PathSetAccessControlRecursiveMode::Modify, acls, options);
    }

    /**
     * @brief Removes POSIX access control rights on files and directories under given directory
     * recursively.
     * @param acls Removes POSIX access control rights on files and directories. Each access control
     * entry (ACE) consists of a scope, a type, a user or group identifier, and permissions.
     * @param options Optional parameters to set an access control recursively to the resource the
     * directory points to.
     * @return
     * Azure::Core::Response<Models::RemoveDataLakeDirectoryAccessControlRecursiveSinglePageResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::RemoveDataLakeDirectoryAccessControlRecursiveSinglePageResult>
    RemoveAccessControlRecursiveSinglePage(
        std::vector<Models::Acl> acls,
        const RemoveDataLakeDirectoryAccessControlRecursiveSinglePageOptions& options
        = RemoveDataLakeDirectoryAccessControlRecursiveSinglePageOptions()) const
    {
      return SetAccessControlRecursiveSinglePageInternal(
          Models::PathSetAccessControlRecursiveMode::Remove, acls, options);
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

    Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveSinglePageResult>
    SetAccessControlRecursiveSinglePageInternal(
        Models::PathSetAccessControlRecursiveMode mode,
        std::vector<Models::Acl> acls,
        const SetDataLakeDirectoryAccessControlRecursiveSinglePageOptions& options
        = SetDataLakeDirectoryAccessControlRecursiveSinglePageOptions()) const;

    friend class DataLakeFileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

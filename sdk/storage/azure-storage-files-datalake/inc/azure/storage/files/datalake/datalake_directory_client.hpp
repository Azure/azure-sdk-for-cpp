// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DirectoryClient : public PathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param directoryPath The path of a directory within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return DirectoryClient
     */
    static DirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& directoryPath,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& directoryUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param credential The client secret credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& directoryUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param directoryUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& directoryUri,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Create a FileClient from current DirectoryClient
     * @param path Path of the file under the directory.
     * @return FileClient
     */
    FileClient GetFileClient(const std::string& path) const;

    /**
     * @brief Create a DirectoryClient from current DirectoryClient
     * @param path Path of the directory under the current directory.
     * @return DirectoryClient
     */
    DirectoryClient GetSubDirectoryClient(const std::string& path) const;

    /**
     * @brief Gets the directory's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The directory's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobClient.GetUri(); }

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
     * @return Azure::Core::Response<CreateDirectoryResult> containing the information of the
     * created directory
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<CreateDirectoryResult> Create(
        const CreateDirectoryOptions& options = CreateDirectoryOptions()) const
    {
      return PathClient::Create(PathResourceType::Directory, options);
    }

    /**
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationDirectoryPath The destinationPath this current directory is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination
     * directory points to.
     * @return Azure::Core::Response<RenameDirectoryResult> containing the information returned when
     * renaming the directory.
     * @remark This operation will not change the URL this directory client points too, to use the
     *         new name, customer needs to initialize a new directory client with the new name/path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<RenameDirectoryResult> Rename(
        const std::string& destinationDirectoryPath,
        const RenameDirectoryOptions& options = RenameDirectoryOptions()) const;

    /**
     * @brief Deletes the directory.
     * @param Recursive If "true", all paths beneath the directory will be deleted. If "false" and
     *                  the directory is non-empty, an error occurs.
     * @param options Optional parameters to delete the directory the path points to.
     * @return Azure::Core::Response<DeleteDirectoryResult> containing the information returned when
     * deleting the directory.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DeleteDirectoryResult> Delete(
        bool Recursive,
        const DeleteDirectoryOptions& options = DeleteDirectoryOptions()) const;

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
     * @return Azure::Core::Response<SetDirectoryAccessControlRecursiveResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<SetDirectoryAccessControlRecursiveResult> SetAccessControlRecursive(
        PathSetAccessControlRecursiveMode mode,
        std::vector<Acl> acls,
        const SetDirectoryAccessControlRecursiveOptions& options
        = SetDirectoryAccessControlRecursiveOptions()) const;

  private:
    explicit DirectoryClient(
        Azure::Core::Http::Url dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : PathClient(std::move(dfsUri), std::move(blobClient), pipeline)
    {
    }
    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

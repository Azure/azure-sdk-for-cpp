// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "credentials/policy/policies.hpp"
#include "datalake/path_client.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  struct DirectoryRenameResponse
  {
    std::string Date;
    Azure::Core::Nullable<std::string> ETag;
    Azure::Core::Nullable<std::string> LastModified;
    std::string RequestId;
    std::string Version;
    Azure::Core::Nullable<std::string> ClientRequestId;
    Azure::Core::Nullable<std::string> Continuation;
  };

  using DirectorySetAccessControlRecursiveResponse = PathSetAccessControlRecursiveResponse;
  using DirectoryInfo = PathInfo;
  using DirectoryCreateOptions = PathCreateOptions;
  using DirectoryDeleteResponse = PathDeleteResponse;

  class DirectoryClient : public PathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param path The path of a resource within the file system.
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
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& directoryUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
     * @brief Gets the directory's primary uri endpoint. This is the endpoint used for blob
     * service interop.
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
    std::string GetDfsUri() const { return m_dfsUri.ToString(); }

    /**
     * @brief Sets POSIX access control rights on files and directories under given directory
     *        recursively.
     * @param mode Mode PathSetAccessControlRecursiveMode::Set sets POSIX access control rights on
     *             files and directories, PathSetAccessControlRecursiveMode::Modify modifies one or
     *             more POSIX access control rights  that pre-exist on files and directories,
     *             PathSetAccessControlRecursiveMode::Remove removes one or more POSIX access
     *             control rights that were present earlier on files and directories
     * @param acls Sets POSIX access control rights on files and directories. Each access control
     *             entry (ACE) consists of a scope, a type, a user or group identifier, and
     *             permissions.
     * @param options Optional parameters to set an access control recursively to the resource the
     *                directory points to.
     * @return PathSetAccessControlRecursiveResponse
     * @remark This request is sent to dfs endpoint.
     */
    DirectorySetAccessControlRecursiveResponse SetAccessControlRecursive(
        PathSetAccessControlRecursiveMode mode,
        std::vector<Acl> acls,
        const SetAccessControlRecursiveOptions& options = SetAccessControlRecursiveOptions()) const;

    /**
     * @brief Create a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the directory the path points to.
     * @return PathInfo
     * @remark This request is sent to dfs endpoint.
     */
    DirectoryInfo Create(const DirectoryCreateOptions& options = DirectoryCreateOptions()) const
    {
      return PathClient::Create(PathResourceType::Directory, options);
    }

    /**
     * @brief Renames a directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationDirectoryPath The destinationPath this current directory is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination
     * directory points to.
     * @return DirectoryRenameResponse
     * @remark This will change the URL the client is pointing to.
     * @remark This request is sent to dfs endpoint.
     */
    DirectoryRenameResponse Rename(
        const std::string& destinationDirectoryPath,
        const DirectoryRenameOptions& options = DirectoryRenameOptions());

    /**
     * @brief Deletes the directory.
     * @param options Optional parameters to delete the directory the path points to.
     * @return DirectoryDeleteResponse
     * @remark This request is sent to dfs endpoint.
     */
    DirectoryDeleteResponse Delete(
        const DirectoryDeleteOptions& options = DirectoryDeleteOptions()) const;

  private:
    explicit DirectoryClient(
        UriBuilder dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : PathClient(dfsUri, blobClient, pipeline)
    {
    }
    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

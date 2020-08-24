// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "azure/core/credentials/credentials.hpp"
#include "datalake/datalake_file_system_client.hpp"
#include "datalake_options.hpp"
#include "datalake_responses.hpp"
#include "azure/core/http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"
#include "azure/core/response.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class PathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param path The path of a resource within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return PathClient
     */
    static PathClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& path,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param pathUri The URI of the path this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param pathUri The URI of the path this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param pathUri The URI of the path this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Gets the path's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobClient.GetUri(); }

    /**
     * @brief Gets the path's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.ToString(); }

    /**
     * @brief Creates a file or directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<CreatePathResult> containing the information returned when
     * creating a path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<CreatePathResult> Create(
        PathResourceType type,
        const CreatePathOptions& options = CreatePathOptions()) const;

    /**
     * @brief Deletes the resource the path points to.
     * @param options Optional parameters to delete the reource the path points to.
     * @return Azure::Core::Response<DeletePathResult> which is current empty but preserved for
     * future usage.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<DeletePathResult> Delete(
        const DeletePathOptions& options = DeletePathOptions()) const;

    /**
     * @brief Sets the owner, group, permissions, or access control list for a file or directory.
     *        Note that Hierarchical Namespace must be enabled for the account in order to use
     *        access control. Also note that the Access Control List (ACL) includes permissions for
     *        the owner, owning group, and others, so the x-ms-permissions and x-ms-acl request
     *        headers are mutually exclusive.
     * @param acls Sets POSIX access control rights on files and directories. Each access control
     *             entry (ACE) consists of a scope, a type, a user or group identifier, and
     *             permissions.
     * @param options Optional parameters to set an access control to the resource the path points
     *                to.
     * @return Azure::Core::Response<SetPathAccessControlResult> containing the information returned
     * when setting path's access control.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<SetPathAccessControlResult> SetAccessControl(
        std::vector<Acl> acls,
        const SetPathAccessControlOptions& options = SetPathAccessControlOptions()) const;

    /**
     * @brief Sets the properties of a resource the path points to.
     * @param options Optional parameters to set the http headers to the resource the path points
     * to.
     * @return Azure::Core::Response<SetPathHttpHeadersResult> containing the information returned
     * when setting the path's Http headers.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<SetPathHttpHeadersResult> SetHttpHeaders(
        DataLakeHttpHeaders httpHeaders,
        const SetPathHttpHeadersOptions& options = SetPathHttpHeadersOptions()) const;

    /**
     * @brief Get Properties returns all system and user defined properties for a path. Get Status
     *        returns all system defined properties for a path. Get Access Control List returns the
     *        access control list for a path.
     * @param options Optional parameters to get the properties from the resource the path points
     *                to.
     * @return Azure::Core::Response<GetPathPropertiesResult> containing the properties of the path.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<GetPathPropertiesResult> GetProperties(
        const GetPathPropertiesOptions& options = GetPathPropertiesOptions()) const;

    /**
     * @brief Returns all access control list stored for the given path.
     * @param options Optional parameters to get the ACLs from the resource the path points to.
     * @return Azure::Core::Response<GetPathAccessControlResult> containing the access control list
     * of the path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<GetPathAccessControlResult> GetAccessControls(
        const GetPathAccessControlOptions& options = GetPathAccessControlOptions()) const;

    /**
     * @brief Sets the metadata of a resource the path points to.
     * @param options Optional parameters to set the metadata to the resource the path points to.
     * @return Azure::Core::Response<SetPathMetadataResult> containing the information returned when
     * setting the metadata.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<SetPathMetadataResult> SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const SetPathMetadataOptions& options = SetPathMetadataOptions()) const;

  protected:
    UriBuilder m_dfsUri;
    Blobs::BlobClient m_blobClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit PathClient(
        UriBuilder dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_dfsUri(std::move(dfsUri)), m_blobClient(std::move(blobClient)),
          m_pipeline(std::move(pipeline))
    {
    }

    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

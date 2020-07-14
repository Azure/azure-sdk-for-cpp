// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "datalake/file_system_client.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  struct Acl
  {
    std::string Scope;
    std::string Type;
    std::string Id;
    std::string Permissions;

    /**
     * @brief Creates an Acl based on acl input string.
     * @param aclString the string to be parsed to Acl.
     * @return Acl
     */
    static Acl FromString(const std::string& aclString);

    /**
     * @brief Creates a string from an Acl.
     * @param acl the acl object to be serialized to a string.
     * @return std::string
     */
    static std::string ToString(const Acl& acl);

    /**
     * @brief Creates a vector of Acl from a string that indicates multiple acls.
     * @param dataLakeAclsString the string that contains multiple acls.
     * @return std::vector<Acl>
     */
    static std::vector<Acl> DeserializeAcls(const std::string& dataLakeAclsString);

    /**
     * @brief Creates a string that contains several Acls.
     * @param dataLakeAclsArray the acls to be serialized into a string.
     * @return std::string
     */
    static std::string SerializeAcls(const std::vector<Acl>& dataLakeAclsArray);
  };

  struct GetPathPropertiesResponse
  {
    Azure::Core::Nullable<std::string> AcceptRanges;
    DataLakeHttpHeaders HttpHeaders;
    int64_t ContentLength = int64_t();
    std::string ContentType;
    Azure::Core::Nullable<std::string> ContentMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    Azure::Core::Nullable<std::string> ResourceType;
    Azure::Core::Nullable<std::string> Owner;
    Azure::Core::Nullable<std::string> Group;
    Azure::Core::Nullable<std::string> Permissions;
    Azure::Core::Nullable<std::vector<Acl>> Acls;
    Azure::Core::Nullable<std::string> LeaseDuration;
    LeaseStateType LeaseState;
    LeaseStatusType LeaseStatus;
    std::map<std::string, std::string> Metadata;
  };

  struct SetPathHttpHeadersResponse
  {
    std::string Date;
    std::string RequestId;
    std::string Version;
    std::string ETag;
    std::string LastModified;
    DataLakeHttpHeaders HttpHeaders;
    int64_t ContentLength = int64_t();
    Azure::Core::Nullable<std::string> ContentRange;
    Azure::Core::Nullable<std::string> ContentMD5;
    Azure::Core::Nullable<std::string> Continuation;
    int32_t DirectoriesSuccessful = int32_t();
    int32_t FilesSuccessful = int32_t();
    int32_t FailureCount = int32_t();
    std::vector<AclFailedEntry> FailedEntries;
  };

  struct SetPathMetadataResponse
  {
    std::string Date;
    std::string RequestId;
    std::string Version;
    std::string ETag;
    std::string LastModified;
  };

  struct PathInfo
  {
    std::string Date;
    Azure::Core::Nullable<std::string> ETag;
    Azure::Core::Nullable<std::string> LastModified;
    std::string RequestId;
    std::string Version;
    Azure::Core::Nullable<int64_t> ContentLength;
  };

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
        std::shared_ptr<TokenCredential> credential,
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
     * service interop.
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobUri.ToString(); }

    /**
     * @brief Gets the path's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.ToString(); }

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
     * @return PathFlushDataResponse
     */
    PathSetAccessControlResponse SetAccessControl(
        std::vector<Acl> acls,
        const SetAccessControlOptions& options = SetAccessControlOptions()) const;

    /**
     * @brief Sets the properties of a resource the path points to.
     * @param options Optional parameters to set the http headers to the resource the path points
     * to.
     * @return SetPathHttpHeadersResponse
     */
    SetPathHttpHeadersResponse SetHttpHeaders(
        DataLakeHttpHeaders httpHeaders,
        const SetPathHttpHeadersOptions& options = SetPathHttpHeadersOptions()) const;

    /**
     * @brief Get Properties returns all system and user defined properties for a path. Get Status
     *        returns all system defined properties for a path. Get Access Control List returns the
     *        access control list for a path.
     * @param options Optional parameters to get the properties from the resource the path points
     *                to.
     * @return GetPathPropertiesResponse
     */
    GetPathPropertiesResponse GetProperties(
        const PathGetPropertiesOptions& options = PathGetPropertiesOptions()) const;

    /**
     * @brief Sets the metadata of a resource the path points to.
     * @param options Optional parameters to set the metadata to the resource the path points to.
     * @return SetPathMetadataResponse
     */
    SetPathMetadataResponse SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const SetPathMetadataOptions& options = SetPathMetadataOptions()) const;

  protected:
    /**
     * @brief Creates a file or directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return PathInfo
     */
    PathInfo Create(PathResourceType type, const PathCreateOptions& options = PathCreateOptions())
        const;

    UriBuilder m_dfsUri;
    UriBuilder m_blobUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
    explicit PathClient(
        UriBuilder dfsUri,
        UriBuilder blobUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_dfsUri(std::move(dfsUri)), m_blobUri(std::move(blobUri)),
          m_pipeline(std::move(pipeline))
    {
    }
    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake

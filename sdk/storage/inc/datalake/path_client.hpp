// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "datalake/file_system_client.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace DataLake {

  struct Acl
  {
    std::string Scope;
    std::string Type;
    std::string Id;
    std::string Permissions;

    static Acl FromString(const std::string& aclString);
    static std::string ToString(const Acl& acl);
    static std::vector<Acl> DeserializeAcls(const std::string& dataLakeAclsString);
    static std::string SerializeAcls(const std::vector<Acl>& dataLakeAclsArray);
  };

  struct ReadPathResponse
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> Body;
    std::string AcceptRanges;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    int64_t ContentLength = int64_t();
    int64_t RangeOffset = int64_t();
    int64_t RangeLength = int64_t();
    std::string ContentType;
    std::string ContentMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string ResourceType;
    std::string LeaseDuration;
    std::string LeaseState;
    std::string LeaseStatus;
    std::string ContentMd5;
    std::map<std::string, std::string> Metadata;
  };

  struct GetPathPropertiesResponse
  {
    std::string AcceptRanges;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    int64_t ContentLength = int64_t();
    int64_t RangeOffset = int64_t();
    int64_t RangeLength = int64_t();
    std::string ContentType;
    std::string ContentMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string ResourceType;
    std::string Owner;
    std::string Group;
    std::string Permissions;
    std::vector<Acl> Acls;
    std::string LeaseDuration;
    std::string LeaseState;
    std::string LeaseStatus;
    std::map<std::string, std::string> Metadata;
  };

  typedef PathCreateResponse PathRenameResponse;

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
     * @param pathUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param pathUri The URI of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        std::shared_ptr<TokenCredential> credential,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param pathUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit PathClient(
        const std::string& pathUri,
        const PathClientOptions& options = PathClientOptions());

    /**
     * @brief Uploads data to be appended to a file. Data can only be appended to a file.
     * @param data The data to be appended.
     * @param offset This parameter allows the caller to upload data in parallel and control
     *                 the order in which it is appended to the file.
     *                 The value must be the offset where the data is to be appended.
     *                 Uploaded data is not immediately flushed, or written, to the file. To flush,
     *                 the previously uploaded data must be contiguous, the offset parameter must
     *                 be specified and equal to the length of the file after all data has been
     *                 written, and there must not be a request entity body included with the
     *                 request.
     * @param options Optional parameters to append data to the resource the path points to.
     * @return PathAppendDataResponse
     */
    PathAppendDataResponse AppendData(
        std::unique_ptr<Azure::Core::Http::BodyStream> stream,
        int64_t offset,
        const PathAppendDataOptions& options = PathAppendDataOptions()) const;

    /**
     * @brief Flushes previous uploaded data to a file.
     * @param offset This parameter allows the caller to upload data in parallel and control
     *                 the order in which it is appended to the file.
     *                 The value must be the offset where the data is to be appended.
     *                 Uploaded data is not immediately flushed, or written, to the file. To flush,
     *                 the previously uploaded data must be contiguous, the offset parameter must
     *                 be specified and equal to the length of the file after all data has been
     *                 written, and there must not be a request entity body included with the
     *                 request.
     * @param options Optional parameters to flush data to the resource the path points to.
     * @return PathFlushDataResponse
     */
    PathFlushDataResponse FlushData(
        int64_t offset,
        const PathFlushDataOptions& options = PathFlushDataOptions()) const;

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
     * @brief Sets POSIX access control rights on files and directories under given path
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
     *                path points to.
     * @return PathSetAccessControlRecursiveResponse
     */
    PathSetAccessControlRecursiveResponse SetAccessControlRecursive(
        PathSetAccessControlRecursiveMode mode,
        std::vector<Acl> acls,
        const SetAccessControlRecursiveOptions& options = SetAccessControlRecursiveOptions()) const;

    /**
     * @brief Sets the properties of a resource the path points to.
     * @param options Optional parameters to set the properties to the resource the path points to.
     * @return PathUpdateResponse
     */
    PathUpdateResponse SetProperties(
        const SetPathPropertiesOptions& options = SetPathPropertiesOptions()) const;

    /**
     * @brief Creates a file or directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return PathCreateResponse
     */
    PathCreateResponse Create(const PathCreateOptions& options = PathCreateOptions()) const;

    /**
     * @brief Renames a file or directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationPath The destinationPath this path is renaming to.
     * @param options Optional parameters to rename a resource to the resource the path points to.
     * @return PathRenameResponse
     * @remark This will change the URL the client is pointing to.
     */
    PathRenameResponse Rename(
        const std::string& destinationPath,
        const PathRenameOptions& options = PathRenameOptions());

    /**
     * @brief Deletes the file or directory.
     * @param options Optional parameters to delete the resource the path points to.
     * @return PathDeleteResponse
     */
    PathDeleteResponse Delete(const PathDeleteOptions& options = PathDeleteOptions()) const;

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

    // TODO: Remove or uncomment after finalized how to support lease.
    ///**
    // * @brief Create and manage a lease to restrict write and delete access to the path.
    // * @param options Optional parameters to create or manage a lease on the resource the path
    // *                points to.
    // * @return PathLeaseResponse
    // */
    // PathLeaseResponse Lease(const PathLeaseOptions& options = PathLeaseOptions()) const;

    /**
     * @brief Read the contents of a file. For read operations, range requests are supported.
     * @param options Optional parameters to read the content from the resource the path points to.
     * @return ReadPathResponse
     */
    ReadPathResponse Read(const PathReadOptions& options = PathReadOptions()) const;

  private:
    UrlBuilder m_dfsUri;
    UrlBuilder m_blobUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    PathClient() = default;
    friend class FileSystemClient;
  };
}}} // namespace Azure::Storage::DataLake

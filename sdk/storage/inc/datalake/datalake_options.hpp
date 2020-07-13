// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/access_conditions.hpp"
#include "common/shared_request_options.hpp"
#include "nullable.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace DataLake {

  /**
   * @brief Service client options used to initalize DataLakeServiceClient.
   */
  struct ServiceClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;
  };

  /**
   * @brief File system client options used to initalize DataLakeFileSystemClient.
   */
  struct FileSystemClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;
  };

  /**
   * @brief Path client options used to initalize DataLakePathClient.
   */
  struct PathClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;
  };

  /**
   * @brief Specifies access conditions for a file system.
   */
  struct FileSystemAccessConditions : public LastModifiedTimeAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for a path.
   */
  struct PathAccessConditions : public LastModifiedTimeAccessConditions,
                                public ETagAccessConditions,
                                public LeaseAccessConditions
  {
  };

  /**
   * @brief Optional parameters for DataLakeServiceClient::ListFilesSystems
   */
  struct ListFileSystemsOptions : public SharedRequestOptions
  {
    /**
     * @brief Filters results to filesystems within the specified prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief The number of filesystems returned with each invocation is
     *        limited. If the number of filesystems to be returned exceeds
     *        this limit, a continuation token is returned in the response
     *        header x-ms-continuation. When a continuation token is returned
     *        in the response, it must be specified in a subsequent invocation
     *        of the list operation to continue listing the filesystems.
     */
    Azure::Core::Nullable<std::string> Continuation;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::Create
   */
  struct FileSystemCreateOptions : public SharedRequestOptions
  {
    /**
     * @brief User-defined metadata to be stored with the filesystem.
     *        Note that the string may only contain ASCII characters in the
     *        ISO-8859-1 character set.
     */
    std::map<std::string, std::string> Metadata;
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::Delete
   */
  struct FileSystemDeleteOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the access condition for the file system.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::GetProperties
   */
  struct FileSystemGetPropertiesOptions : public SharedRequestOptions
  {
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::SetMetadata
   */
  struct FileSystemSetMetadataOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the access condition for the file system.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::ListPaths
   */
  struct ListPathsOptions : public SharedRequestOptions
  {
    /**
     * @brief Valid only when Hierarchical Namespace is enabled for the account.
     *        If "true", the user identity values returned in the owner and group
     *        fields of each list entry will be transformed from Azure Active Directory
     *        Object IDs to User Principal Names. If "false", the values will be
     *        returned as Azure Active Directory Object IDs. The default value is false.
     *        Note that group and application Object IDs are not translated because they
     *        do not have unique friendly names.
     */
    Azure::Core::Nullable<bool> UserPrincipalName;

    /**
     * @brief The number of paths returned with each invocation is
     *        limited. If the number of paths to be returned exceeds
     *        this limit, a continuation token is returned in the response
     *        header x-ms-continuation. When a continuation token is returned
     *        in the response, it must be specified in a subsequent invocation
     *        of the list operation to continue listing the paths.
     */
    Azure::Core::Nullable<std::string> Continuation;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Filters results to paths within the specified directory. An error occurs
     *        if the directory does not exist.
     */
    Azure::Core::Nullable<std::string> Directory;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::AppendData
   */
  struct PathAppendDataOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the transactional md5 for the body, to be validated by the service.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief The lease ID must be specified if there is an active lease.
     */
    Azure::Core::Nullable<std::string> LeaseId;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::FlushData
   */
  struct PathFlushDataOptions : public SharedRequestOptions
  {
    /**
     * @brief If "true", uncommitted data is retained after the flush operation completes;
     *        otherwise, the uncommitted data is deleted after the flush operation.  The
     *        default is false.  Data at offsets less than the specified position are
     *        written to the file when flush succeeds, but this optional parameter allows
     *        data after the flush position to be retained for a future flush operation.
     */
    Azure::Core::Nullable<bool> RetainUncommittedData;

    /**
     * @brief Azure Storage Events allow applications to receive notifications when files
     *        change. When Azure Storage Events are enabled, a file changed event is raised.
     *        This event has a property indicating whether this is the final change to distinguish
     *        the difference between an intermediate flush to a file stream and the final close of
     *        a file stream. The close query parameter is valid only when the action is "flush"
     *        and change notifications are enabled. If the value of close is "true" and the
     *        flush operation completes successfully, the service raises a file change notification
     *        with a property indicating that this is the final update (the file stream has been
     *        closed). If "false" a change notification is raised indicating the file has changed.
     *        The default is false. This query parameter is set to true by the Hadoop ABFS driver to
     *        indicate that the file stream has been closed."
     */
    Azure::Core::Nullable<bool> Close;

    /**
     * @brief The service stores this value and includes it in the "Content-Md5" response header for
     *        "Read & Get Properties" operations. If this property is not specified on the request,
     *        then the property will be cleared for the file. Subsequent calls to "Read & Get
     *        Properties" will not return this property unless it is explicitly set on that file
     *        again.
     */
    Azure::Core::Nullable<std::string> ContentMD5;

    /**
     * @brief Specify the http headers for this path.
     */
    Azure::Storage::DataLake::DataLakeHttpHeaders HttpHeaders;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetAccessControl
   */
  struct SetAccessControlOptions : public SharedRequestOptions
  {
    /**
     * @brief The owner of the path or directory.
     */
    Azure::Core::Nullable<std::string> Owner;

    /**
     * @brief The owning group of the path or directory.
     */
    Azure::Core::Nullable<std::string> Group;

    /**
     * @brief only valid if Hierarchical Namespace is enabled for the account. Sets POSIX
     *        access permissions for the file owner, the file owning group, and others.
     *        Each class may be granted read, write, or execute permission.
     *        The sticky bit is also supported.  Both symbolic (rwxrw-rw-) and 4-digit octal
     *        notation (e.g. 0766) are supported.
     */
    Azure::Core::Nullable<std::string> Permissions;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetAccessControlRecursive
   */
  struct SetAccessControlRecursiveOptions : public SharedRequestOptions
  {
    /**
     * @brief When performing setAccessControlRecursive on a directory, the number of paths that
     *        are processed with each invocation is limited.  If the number of paths to be processed
     *        exceeds this limit, a continuation token is returned in this response header.  When a
     *        continuation token is returned in the response, it must be specified in a subsequent
     *        invocation of the setAccessControlRecursive operation to continue the
     *        setAccessControlRecursive operation on the directory.
     */
    Azure::Core::Nullable<std::string> Continuation;

    /**
     * @brief It specifies the maximum number of files or directories on which the acl change will
     *        be applied. If omitted or greater than 2,000, the request will process up to 2,000
     *        items.
     */
    Azure::Core::Nullable<int32_t> MaxRecords;

    /**
     * @brief Sets POSIX access control rights on files and directories. The value is a
     *        comma-separated list of access control entries. Each access control entry (ACE)
     *        consists of a scope, a type, a user or group identifier, and permissions in the format
     *        "[scope:][type]:[id]:[permissions]".
     */
    Azure::Core::Nullable<std::string> Acl;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetHttpHeaders
   */
  struct SetPathHttpHeadersOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetMetadata
   */
  struct SetPathMetadataOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::Create
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct PathCreateOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the http headers for this path.
     */
    Azure::Storage::DataLake::DataLakeHttpHeaders HttpHeaders;

    /**
     * @brief User-defined metadata to be stored with the path. Note that the string may only
     *        contain ASCII characters in the ISO-8859-1 character set.  If the filesystem exists,
     *        any metadata not included in the list will be removed.  All metadata are removed
     *        if the header is omitted.  To merge new and existing metadata, first get all
     *        existing metadata and the current E-Tag, then make a conditional request with the
     *        E-Tag and include values for all metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Only valid if Hierarchical Namespace is enabled for the account. When creating
     *        a file or directory and the parent folder does not have a default ACL, the umask
     *        restricts the permissions of the file or directory to be created.  The resulting
     *        permission is given by p bitwise and not u, where p is the permission and u is
     *        the umask.  For example, if p is 0777 and u is 0057, then the resulting permission
     *        is 0720.  The default permission is 0777 for a directory and 0666 for a file.
     *        The default umask is 0027.  The umask must be specified in 4-digit octal
     *        notation (e.g. 0766).
     */
    Azure::Core::Nullable<std::string> Umask;

    /**
     * @brief only valid if Hierarchical Namespace is enabled for the account. Sets POSIX
     *        access permissions for the file owner, the file owning group, and others.
     *        Each class may be granted read, write, or execute permission.
     *        The sticky bit is also supported.  Both symbolic (rwxrw-rw-) and 4-digit octal
     *        notation (e.g. 0766) are supported.
     */
    Azure::Core::Nullable<std::string> Permissions;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::Create
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct PathRenameOptions : public SharedRequestOptions
  {
    /**
     * @brief When renaming a directory, the number of paths that are renamed with each
     *        invocation is limited. If the number of paths to be renamed exceeds this limit,
     *        a continuation token is returned in this response header. When a continuation token
     *        is returned in the response, it must be specified in a subsequent invocation of the
     *        rename operation to continue renaming the directory.
     */
    Azure::Core::Nullable<std::string> Continuation;

    /**
     * @brief Valid only when namespace is enabled. This parameter determines the behavior of the
     *        rename operation. The value must be PathRenameMode::Legacy or PathRenameMode::Posix,
     *        and the default value will be PathRenameMode::Posix.
     */
    PathRenameMode Mode = PathRenameMode::Posix;

    /**
     * @brief If not specified, the source's file system is used. Otherwise, rename to destination
     *        file system.
     */
    Azure::Core::Nullable<std::string> DestinationFileSystem;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;

    /**
     * @brief The access condition for source path.
     */
    PathAccessConditions SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::Delete
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/delete
   */
  struct PathDeleteOptions : public SharedRequestOptions
  {
    /**
     * @brief When deleting a directory, the number of paths that are deleted with each invocation
     *        is limited. If the number of paths to be deleted exceeds this limit, a continuation
     *        token is returned in this response header.  When a continuation token is returned in
     *        the response, it must be specified in a subsequent invocation of the delete operation
     *        to continue deleting the directory.
     */
    Azure::Core::Nullable<std::string> Continuation;

    /**
     * @brief Required and valid only when the resource is a directory. If "true", all paths beneath
     *        the directory will be deleted. If "false" and the directory is non-empty, an error
     *        occurs.
     */
    Azure::Core::Nullable<bool> RecursiveOptional;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::GetProperties
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/getproperties
   */
  struct PathGetPropertiesOptions : public SharedRequestOptions
  {
    /**
     * @brief If the value is PathGetPropertiesAction::GetStatus only the system defined properties
     *        for the path are returned. If the value is PathGetPropertiesAction::GetAccessControl
     *        the access control list is returned in the response headers (Hierarchical Namespace
     *        must be enabled for the account), otherwise the properties are returned.
     */
    Azure::Core::Nullable<PathGetPropertiesAction> Action;

    /**
     * @brief Valid only when Hierarchical Namespace is enabled for the account. If "true",
     *        the user identity values returned in the x-ms-owner, x-ms-group, and x-ms-acl
     *        response headers will be transformed from Azure Active Directory Object IDs to
     *        User Principal Names. If "false", the values will be returned as Azure Active
     *        Directory Object IDs. The default value is false. Note that group and application
     *        Object IDs are not translated because they do not have unique friendly names.
     */
    Azure::Core::Nullable<bool> UserPrincipalName;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::Read
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/read
   */
  struct PathReadOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the offset of the starting range to be retrieved.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief Specify the length to be retreived if an offset has been specified.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief When this header is set to "true" and specified together with the Range header,
     *        the service returns the MD5 hash for the range, as long as the range is less than
     *        or equal to 4MB in size. If this header is specified without the Range header,
     *        the service returns status code 400 (Bad Request). If this header is set to true
     *        when the range exceeds 4 MB in size, the service returns status code 400 (Bad
     *        Request).
     */
    Azure::Core::Nullable<bool> RangeGetContentMd5;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };
}}} // namespace Azure::Storage::DataLake

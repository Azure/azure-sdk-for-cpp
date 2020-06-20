// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/shared_request_options.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace DataLake {

  /**
   * @brief Service client options used to initalize DataLakeServiceClient.
   */
  struct ServiceClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief File system client options used to initalize DataLakeFileSystemClient.
   */
  struct FileSystemClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief Path client options used to initalize DataLakePathClient.
   */
  struct PathClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  };

  /**
   * @brief Optional parameters for DataLakeServiceClient::ListFilesSystems
   */
  struct ListFileSystemsOptions : public SharedRequestOptions
  {
    /**
     * @brief Filters results to filesystems within the specified prefix.
     */
    std::string Prefix;

    /**
     * @brief The number of filesystems returned with each invocation is
     *        limited. If the number of filesystems to be returned exceeds
     *        this limit, a continuation token is returned in the response
     *        header x-ms-continuation. When a continuation token is returned
     *        in the response, it must be specified in a subsequent invocation
     *        of the list operation to continue listing the filesystems.
     */
    std::string Continuation;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    int32_t MaxResults = 0;
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
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::GetMetadata
   */
  struct FileSystemGetMetadataOptions : public SharedRequestOptions
  {
  };

  /**
   * @brief Optional parameters for DataLakeFileSystemClient::SetMetadata
   */
  struct FileSystemSetMetadataOptions : public SharedRequestOptions
  {
    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
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
    bool Upn = bool();

    /**
     * @brief The number of paths returned with each invocation is
     *        limited. If the number of paths to be returned exceeds
     *        this limit, a continuation token is returned in the response
     *        header x-ms-continuation. When a continuation token is returned
     *        in the response, it must be specified in a subsequent invocation
     *        of the list operation to continue listing the paths.
     */
    std::string Continuation;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    int32_t MaxResults = 0;

    /**
     * @brief Filters results to paths within the specified directory. An error occurs
     *        if the directory does not exist.
     */
    std::string Directory;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::AppendData
   */
  struct PathAppendDataOptions : public SharedRequestOptions
  {
    /**
     * @brief Specify the transactional md5 for the body, to be validated by the service.
     */
    std::string ContentMD5;

    /**
     * @brief The lease ID must be specified if there is an active lease.
     */
    std::string LeaseId;
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
    bool RetainUncommittedData = bool();

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
    bool Close = bool();

    /**
     * @brief The service stores this value and includes it in the "Content-Md5" response header for
     *        "Read & Get Properties" operations. If this property is not specified on the request,
     *        then the property will be cleared for the file. Subsequent calls to "Read & Get
     *        Properties" will not return this property unless it is explicitly set on that file
     *        again.
     */
    std::string ContentMD5;

    /**
     * @brief The lease ID must be specified if there is an active lease.
     */
    std::string LeaseId;

    /**
     * @brief Sets the path's cache control. If specified, this property is stored with the
     *        path and returned with a read request.
     */
    std::string CacheControl;

    /**
     * @brief Sets the path's content type. If specified, this property is stored
     *        with the path and returned with a read request.
     */
    std::string ContentType;

    /**
     * @brief Sets the path's Content-Disposition header.
     */
    std::string ContentDisposition;

    /**
     * @brief Sets the path's content encoding. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentEncoding;

    /**
     * @brief Set the path's content language. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentLanguage;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetAccessControl
   */
  struct SetAccessControlOptions : public SharedRequestOptions
  {
    /**
     * @brief The lease ID must be specified if there is an active lease.
     */
    std::string LeaseId;

    /**
     * @brief The owner of the path or directory.
     */
    std::string Owner;

    /**
     * @brief The owning group of the path or directory.
     */
    std::string Group;

    /**
     * @brief only valid if Hierarchical Namespace is enabled for the account. Sets POSIX
     *        access permissions for the file owner, the file owning group, and others.
     *        Each class may be granted read, write, or execute permission.
     *        The sticky bit is also supported.  Both symbolic (rwxrw-rw-) and 4-digit octal
     *        notation (e.g. 0766) are supported.
     */
    std::string Permissions;

    /**
     * @brief Sets POSIX access control rights on files and directories. The value is a
     *        comma-separated list of access control entries. Each access control entry (ACE)
     *        consists of a scope, a type, a user or group identifier, and permissions in the
     *        format "[scope:][type]:[id]:[permissions]".
     */
    std::string Acl;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief Specify this header value to operate only on a path if it has been modified
     *        since the specified date/time.
     */
    std::string IfModifiedSince;

    /**
     * @brief Specify this header value to operate only on a path if it has not been modified
     *        since the specified date/time.
     */
    std::string IfUnmodifiedSince;
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
    std::string Continuation;

    /**
     * @brief It specifies the maximum number of files or directories on which the acl change will
     *        be applied. If omitted or greater than 2,000, the request will process up to 2,000
     *        items.
     */
    int32_t MaxRecords = int32_t();

    /**
     * @brief Sets POSIX access control rights on files and directories. The value is a
     *        comma-separated list of access control entries. Each access control entry (ACE)
     *        consists of a scope, a type, a user or group identifier, and permissions in the format
     *        "[scope:][type]:[id]:[permissions]".
     */
    std::string Acl;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::SetProperties
   */
  struct SetPathPropertiesOptions : public SharedRequestOptions
  {
    /**
     * @brief User-defined metadata to be stored with the filesystem.
     *        Note that the string may only contain ASCII characters in the
     *        ISO-8859-1 character set.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Sets the path's cache control. If specified, this property is stored with the
     *        path and returned with a read request.
     */
    std::string CacheControl;

    /**
     * @brief Sets the path's content type. If specified, this property is stored
     *        with the path and returned with a read request.
     */
    std::string ContentType;

    /**
     * @brief Sets the path's Content-Disposition header.
     */
    std::string ContentDisposition;

    /**
     * @brief Sets the path's content encoding. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentEncoding;

    /**
     * @brief Set the path's content language. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentLanguage;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
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
     * @brief Required only for Create File and Create Directory. The value must be
     *        PathResourceType::File or PathResourceType::Directory.
     */
    PathResourceType Resource = PathResourceType::Unknown;

    /**
     * @brief Sets the path's cache control. If specified, this property is stored with the
     *        path and returned with a read request.
     */
    std::string CacheControl;

    /**
     * @brief Sets the path's content type. If specified, this property is stored
     *        with the path and returned with a read request.
     */
    std::string ContentType;

    /**
     * @brief Sets the path's Content-Disposition header.
     */
    std::string ContentDisposition;

    /**
     * @brief Sets the path's content encoding. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentEncoding;

    /**
     * @brief Set the path's content language. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentLanguage;

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;

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
    std::string Umask;

    /**
     * @brief only valid if Hierarchical Namespace is enabled for the account. Sets POSIX
     *        access permissions for the file owner, the file owning group, and others.
     *        Each class may be granted read, write, or execute permission.
     *        The sticky bit is also supported.  Both symbolic (rwxrw-rw-) and 4-digit octal
     *        notation (e.g. 0766) are supported.
     */
    std::string Permissions;
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
     * @brief Required only for Create File and Create Directory. The value must be
     *        PathResourceType::File or PathResourceType::Directory.
     */
    PathResourceType Resource = PathResourceType::Unknown;

    /**
     * @brief When renaming a directory, the number of paths that are renamed with each
     *        invocation is limited. If the number of paths to be renamed exceeds this limit,
     *        a continuation token is returned in this response header. When a continuation token
     *        is returned in the response, it must be specified in a subsequent invocation of the
     *        rename operation to continue renaming the directory.
     */
    std::string Continuation;

    /**
     * @brief Valid only when namespace is enabled. This parameter determines the behavior of the
     *        rename operation. The value must be PathRenameMode::Legacy or PathRenameMode::Posix,
     *        and the default value will be PathRenameMode::Posix.
     */
    PathRenameMode Mode = PathRenameMode::Posix;

    /**
     * @brief Sets the path's cache control. If specified, this property is stored with the
     *        path and returned with a read request.
     */
    std::string CacheControl;

    /**
     * @brief Sets the path's content type. If specified, this property is stored
     *        with the path and returned with a read request.
     */
    std::string ContentType;

    /**
     * @brief Sets the path's Content-Disposition header.
     */
    std::string ContentDisposition;

    /**
     * @brief Sets the path's content encoding. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentEncoding;

    /**
     * @brief Set the path's content language. If specified, this property is stored with
     *        the path and returned with a read request.
     */
    std::string ContentLanguage;

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief An optional file or directory to be renamed.  The value must have the following
     *        format: "/{filesystem}/{path}".  If Properties is specified, the properties
     *        will overwrite the existing properties; otherwise, the existing properties will
     *        be preserved. This value must be a URL percent-encoded string. Note that the string
     *        may only contain ASCII characters in the ISO-8859-1 character set.
     */
    std::string RenameSource;

    /**
     * @brief A lease ID for the source path. If specified, the source path must have an active
     *        lease and the leaase ID must match.
     */
    std::string SourceLeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;

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
    std::string Umask;

    /**
     * @brief only valid if Hierarchical Namespace is enabled for the account. Sets POSIX
     *        access permissions for the file owner, the file owning group, and others.
     *        Each class may be granted read, write, or execute permission.
     *        The sticky bit is also supported.  Both symbolic (rwxrw-rw-) and 4-digit octal
     *        notation (e.g. 0766) are supported.
     */
    std::string Permissions;

    /**
     * @brief Specify an ETag value to operate only on source path with a matching value.
     */
    std::string SourceIfMatch;

    /**
     * @brief Specify an ETag value to operate only on source path without a matching value.
     */
    std::string SourceIfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the source resource has been modified since the
     (        specified date and time.
    */
    std::string SourceIfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the source resource has not been modified since
     *        the specified date and time.
     */
    std::string SourceIfUnmodifiedSince;
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
    std::string Continuation;

    /**
     * @brief Required and valid only when the resource is a directory. If "true", all paths beneath
     *        the directory will be deleted. If "false" and the directory is non-empty, an error
     *        occurs.
     */
    bool RecursiveOptional = bool();

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
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
    PathGetPropertiesAction Action = PathGetPropertiesAction::Unknown;

    /**
     * @brief Valid only when Hierarchical Namespace is enabled for the account. If "true",
     *        the user identity values returned in the x-ms-owner, x-ms-group, and x-ms-acl
     *        response headers will be transformed from Azure Active Directory Object IDs to
     *        User Principal Names. If "false", the values will be returned as Azure Active
     *        Directory Object IDs. The default value is false. Note that group and application
     *        Object IDs are not translated because they do not have unique friendly names.
     */
    bool UserPrincipalName = bool();

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
  };

  /**
   * @brief Optional parameters for DataLakePathClient::Lease
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/lease
   */
  struct PathLeaseOptions : public SharedRequestOptions
  {
    /**
     * @brief There are five lease actions: PathLeaseAction::Acquire, PathLeaseAction::Break,
     *        PathLeaseAction::Change, PathLeaseAction::Renew, and PathLeaseAction::Release.
     *        Use PathLeaseAction::Acquire and specify the ProposedLeaseId and LeaseDuration
     *        to acquire a new lease. Use PathLeaseAction::Break to break an existing lease.
     *        When a lease is broken, the lease break period is allowed to elapse, during
     *        which time no lease operation except break and release can be performed on the file.
     *        When a lease is successfully broken, the response indicates the interval in seconds
     *        until a new lease can be acquired. Use PathLeaseAction::Change and specify the current
     *        lease ID in LeaseId and the new lease ID in ProposedLeaseId to change the lease ID of
     *        an active lease. Use PathLeaseAction::Renew and specify the LeaseId to renew an
     *        existing lease. Use PathLeaseAction::Release and specify the LeaseId to release a
     *        lease.
     */
    PathLeaseAction LeaseAction;

    /**
     * @brief Proposed lease ID, in a GUID string format. The DataLake service returns 400
     *        (Invalid request) if the proposed lease ID is not in the correct format.
     *        See Guid Constructor (String) for a list of valid GUID string formats.
     */
    std::string ProposedLeaseId;

    /**
     * @brief The lease duration is required to acquire a lease, and specifies the duration
     *        of the lease in seconds. The lease duration must be between 15 and 60 seconds
     *        or -1 for infinite lease.
     */
    int32_t LeaseDuration = int32_t();

    /**
     * @brief The lease break period duration is optional to break a lease, and specifies the
     *        break period of the lease in seconds. The lease break duration must be between
     *        0 and 60 seconds.
     */
    int32_t LeaseBreakPeriod = int32_t();

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
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
     * @brief The HTTP Range request header specifies one or more byte ranges of the resource
     *        to be retrieved.
     */
    std::string Range;

    /**
     * @brief When this header is set to "true" and specified together with the Range header,
     *        the service returns the MD5 hash for the range, as long as the range is less than
     *        or equal to 4MB in size. If this header is specified without the Range header,
     *        the service returns status code 400 (Bad Request). If this header is set to true
     *        when the range exceeds 4 MB in size, the service returns status code 400 (Bad
     *        Request).
     */
    bool RangeGetContentMd5 = bool();

    /**
     * @brief If specified, the operation only succeeds if the resource's lease is active and
     *        matches this ID.
     */
    std::string LeaseId;

    /**
     * @brief Specify an ETag value to operate only on path with a matching value.
     */
    std::string IfMatch;

    /**
     * @brief Specify an ETag value to operate only on path without a matching value.
     */
    std::string IfNoneMatch;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has been modified since the
     (        specified date and time.
    */
    std::string IfModifiedSince;

    /**
     * @brief A date and time value. Specify this header to perform the
     *        operation only if the resource has not been modified since
     *        the specified date and time.
     */
    std::string IfUnmodifiedSince;
  };
}}} // namespace Azure::Storage::DataLake

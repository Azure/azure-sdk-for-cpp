// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/nullable.hpp>
#include <azure/storage/blobs/blob_options.hpp>
#include <azure/storage/common/access_conditions.hpp>

#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  using DownloadDataLakeFileToOptions = Blobs::DownloadBlobToOptions;
  using GetUserDelegationKeyOptions = Blobs::GetUserDelegationKeyOptions;

  /**
   * @brief Client options used to initalize DataLakeServiceClient, FileSystemClient, PathClient,
   * FileClient and DirectoryClient.
   */
  struct DataLakeClientOptions
  {
    /**
     * @brief Transport pipeline policies for authentication, additional HTTP headers, etc., that
     * are applied to every request.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;

    /**
     * @brief Transport pipeline policies for authentication, additional HTTP headers, etc., that
     * are applied to every retrial.
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryWithSecondaryOptions RetryOptions;

    /**
     * @brief Customized HTTP client. We're going to use the default one if this is empty.
     */
    Azure::Core::Http::TransportPolicyOptions TransportPolicyOptions;
  };

  /**
   * @brief Specifies access conditions for a file system.
   */
  struct FileSystemAccessConditions : public ModifiedTimeConditions, public LeaseAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for a path.
   */
  struct PathAccessConditions : public ModifiedTimeConditions,
                                public ETagAccessConditions,
                                public LeaseAccessConditions
  {
  };

  /**
   * @brief Optional parameters for ServiceClient::ListFileSystemsSinglePageOptions
   */
  struct ListFileSystemsSinglePageOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

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
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    Azure::Core::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for FileSystemClient::Create
   */
  struct CreateDataLakeFileSystemOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief User-defined metadata to be stored with the filesystem.
     *        Note that the string may only contain ASCII characters in the
     *        ISO-8859-1 character set.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for FileSystemClient::Delete
   */
  struct DeleteDataLakeFileSystemOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the file system.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for FileSystemClient::GetProperties
   */
  struct GetDataLakeFileSystemPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the lease access conditions.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for FileSystemClient::SetMetadata
   */
  struct SetDataLakeFileSystemMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the file system.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for FileSystemClient::ListPathsSinglePage
   */
  struct ListPathsSinglePageOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

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
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief An optional value that specifies the maximum number of items to
     *        return. If omitted or greater than 5,000, the response will
     *        include up to 5,000 items.
     */
    Azure::Core::Nullable<int32_t> PageSizeHint;

    /**
     * @brief Filters results to paths within the specified directory. An error occurs
     *        if the directory does not exist.
     */
    Azure::Core::Nullable<std::string> Directory;
  };

  /**
   * @brief Optional parameters for PathClient::AppendData
   */
  struct AppendDataLakeFileDataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the transactional hash for the body, to be validated by the service.
     */
    Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;

    /**
     * @brief Specify the lease access conditions.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::FlushData
   */
  struct FlushDataLakeFileDataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

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
     * @brief The service stores this value and is returned for "Read & Get Properties" operations.
     *        If this property is not specified on the request, then the property will be cleared
     *        for the file. Subsequent calls to "Read & Get Properties" will not return this
     *        property unless it is explicitly set on that file again.
     */
    Azure::Core::Nullable<Storage::ContentHash> ContentHash;

    /**
     * @brief Specify the http headers for this path.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::SetAccessControl
   */
  struct SetDataLakePathAccessControlOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

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
   * @brief Optional parameters for PathClient::SetHttpHeaders
   */
  struct SetDataLakePathHttpHeadersOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::SetMetadata
   */
  struct SetDataLakePathMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::Create
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct CreateDataLakePathOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the http headers for this path.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * @brief User-defined metadata to be stored with the path. Note that the string may only
     *        contain ASCII characters in the ISO-8859-1 character set.  If the filesystem exists,
     *        any metadata not included in the list will be removed.  All metadata are removed
     *        if the header is omitted.  To merge new and existing metadata, first get all
     *        existing metadata and the current E-Tag, then make a conditional request with the
     *        E-Tag and include values for all metadata.
     */
    Storage::Metadata Metadata;

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
   * @brief Optional parameters for DirectoryClient::Delete
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/delete
   */
  struct DeleteDataLakePathOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief When deleting a directory, the number of paths that are deleted with each invocation
     *        is limited. If the number of paths to be deleted exceeds this limit, a continuation
     *        token is returned in this response header.  When a continuation token is returned in
     *        the response, it must be specified in a subsequent invocation of the delete operation
     *        to continue deleting the directory.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Required and valid only when the resource is a directory. If "true", all paths beneath
     *        the directory will be deleted. If "false" and the directory is non-empty, an error
     *        occurs.
     */
    Azure::Core::Nullable<bool> Recursive;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::GetProperties
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/getproperties
   */
  struct GetDataLakePathPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::GetAccessControl
   */
  struct GetDataLakePathAccessControlOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::Read
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/read
   */
  struct ReadDataLakeFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the range of the resource to be retrieved.
     */
    Azure::Core::Nullable<Core::Http::Range> Range;

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

  /**
   * @brief Optional parameters for FileClient::Create
   */
  struct RenameDataLakeFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief This parameter determines the behavior of the rename operation. The value must be
     *        PathRenameMode::Legacy or PathRenameMode::Posix, and the default value will be
     *        PathRenameMode::Posix.
     */
    Models::PathRenameMode Mode = Models::PathRenameMode::Posix;

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
   * @brief Optional parameters for FileClient::Delete
   */
  struct DeleteDataLakeFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::Create
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/en-us/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct RenameDataLakeDirectoryOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief When renaming a directory, the number of paths that are renamed with each
     *        invocation is limited. If the number of paths to be renamed exceeds this limit,
     *        a continuation token is returned in this response header. When a continuation token
     *        is returned in the response, it must be specified in a subsequent invocation of the
     *        rename operation to continue renaming the directory.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief This parameter determines the behavior of the rename operation. The value must be
     *        PathRenameMode::Legacy or PathRenameMode::Posix, and the default value will be
     *        PathRenameMode::Posix.
     */
    Models::PathRenameMode Mode = Models::PathRenameMode::Posix;

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
   * @brief Optional parameters for DirectoryClient::Delete
   */
  struct DeleteDataLakeDirectoryOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief When deleting a directory, the number of paths that are deleted with each invocation
     *        is limited. If the number of paths to be deleted exceeds this limit, a continuation
     *        token is returned in this response header.  When a continuation token is returned in
     *        the response, it must be specified in a subsequent invocation of the delete operation
     *        to continue deleting the directory.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for PathClient::SetAccessControlRecursive
   */
  struct SetDataLakeDirectoryAccessControlRecursiveOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief When performing setAccessControlRecursive on a directory, the number of paths that
     *        are processed with each invocation is limited.  If the number of paths to be processed
     *        exceeds this limit, a continuation token is returned in this response header.  When a
     *        continuation token is returned in the response, it must be specified in a subsequent
     *        invocation of the setAccessControlRecursive operation to continue the
     *        setAccessControlRecursive operation on the directory.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief It specifies the maximum number of files or directories on which the acl change will
     *        be applied. If omitted or greater than 2,000, the request will process up to 2,000
     *        items.
     */
    Azure::Core::Nullable<int32_t> MaxRecords;

    /**
     * @brief  Optional. Valid for "SetAccessControlRecursive" operation. If set to false, the
     * operation will terminate quickly on encountering user errors (4XX). If true, the operation
     * will ignore user errors and proceed with the operation on other sub-entities of the
     * directory. Continuation token will only be returned when forceFlag is true in case of user
     * errors. If not set the default value is false for this.
     */
    Azure::Core::Nullable<bool> ForceFlag;
  };

  using CreateDataLakeFileOptions = CreateDataLakePathOptions;
  using CreateDataLakeDirectoryOptions = CreateDataLakePathOptions;

  /**
   * @brief Optional parameters for FileClient::UploadFromBuffer and FileClient::UploadFromFile
   */
  struct UploadDataLakeFileFromOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * @brief The maximum number of bytes in a single request.
     */
    Azure::Core::Nullable<int64_t> ChunkSize;

    /**
     * @brief The maximum number of threads that may be used in a parallel transfer.
     */
    int Concurrency = 5;
  };

  using ScheduleDataLakeFileExpiryOriginType = Blobs::Models::ScheduleBlobExpiryOriginType;

  /**
   * @brief Optional parameters for FileClient::UploadFromBuffer and FileClient::UploadFromFile
   */
  struct ScheduleDataLakeFileDeletionOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The expiry time from the specified origin. Only work if ExpiryOrigin is
     * ScheduleFileExpiryOriginType::RelativeToCreation or
     * ScheduleFileExpiryOriginType::RelativeToNow.
     */
    Azure::Core::Nullable<int64_t> TimeToExpireInMs;

    /**
     * @brief The expiry time in RFC1123 format. Only work if ExpiryOrigin is
     * ScheduleFileExpiryOriginType::Absolute.
     */
    Azure::Core::Nullable<std::string> ExpiresOn;
  };

  using AcquireDataLakePathLeaseOptions = Blobs::AcquireBlobLeaseOptions;
  using BreakDataLakePathLeaseOptions = Blobs::BreakBlobLeaseOptions;
  using RenewDataLakePathLeaseOptions = Blobs::RenewBlobLeaseOptions;
  using ReleaseDataLakePathLeaseOptions = Blobs::ReleaseBlobLeaseOptions;
  using ChangeDataLakePathLeaseOptions = Blobs::ChangeBlobLeaseOptions;

}}}} // namespace Azure::Storage::Files::DataLake

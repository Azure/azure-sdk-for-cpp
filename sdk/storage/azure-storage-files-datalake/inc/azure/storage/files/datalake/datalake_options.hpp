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

  namespace Models {
    using ListFileSystemsIncludeFlags = Blobs::Models::ListBlobContainersIncludeFlags;
    using SignedIdentifier = Blobs::Models::SignedIdentifier;
  } // namespace Models

  using DownloadFileToOptions = Blobs::DownloadBlobToOptions;
  using GetUserDelegationKeyOptions = Blobs::GetUserDelegationKeyOptions;

  /**
   * @brief Client options used to initialize all DataLake clients.
   */
  struct DataLakeClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;

    /**
     * API version used by this client.
     */
    std::string ApiVersion = _detail::DefaultServiceApiVersion;
  };

  /**
   * @brief Specifies access conditions for a file system.
   */
  struct FileSystemAccessConditions final : public Azure::ModifiedConditions,
                                            public LeaseAccessConditions
  {
  };

  /**
   * @brief Specifies access conditions for a path.
   */
  struct PathAccessConditions final : public Azure::ModifiedConditions,
                                      public Azure::MatchConditions,
                                      public LeaseAccessConditions
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::ServiceClient::ListFileSystems.
   */
  struct ListFileSystemsOptions final
  {
    /**
     * Filters results to filesystems within the specified prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * The number of filesystems returned with each invocation is limited. If the number of
     * filesystems to be returned exceeds this limit, a continuation token is returned in the
     * response header x-ms-continuation. When a continuation token is returned in the response, it
     * must be specified in a subsequent invocation of the list operation to continue listing the
     * filesystems.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * An optional value that specifies the maximum number of items to return. If omitted or greater
     * than 5,000, the response will include up to 5,000 items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Specifies that the filesystem's metadata be returned.
     */
    Models::ListFileSystemsIncludeFlags Include = Models::ListFileSystemsIncludeFlags::None;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileSystemClient::Create.
   */
  struct CreateFileSystemOptions final
  {
    /**
     * User-defined metadata to be stored with the filesystem. Note that the string may only contain
     * ASCII characters in the ISO-8859-1 character set.
     */
    Storage::Metadata Metadata;

    /**
     * The public access type of the file system.
     */
    Models::PublicAccessType AccessType = Models::PublicAccessType::None;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileSystemClient::Delete.
   */
  struct DeleteFileSystemOptions final
  {
    /**
     * Specify the access condition for the file system.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::FileSystemClient::GetProperties.
   */
  struct GetFileSystemPropertiesOptions final
  {
    /**
     * Specify the lease access conditions.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileSystemClient::SetMetadata.
   */
  struct SetFileSystemMetadataOptions final
  {
    /**
     * Specify the access condition for the file system.
     */
    struct : public LeaseAccessConditions
    {
      /**
       * @brief Specify this header to perform the operation only if the resource has been
       * modified since the specified time. This timestamp will be truncated to second.
       */
      Azure::Nullable<Azure::DateTime> IfModifiedSince;
    } AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileSystemClient::ListPaths.
   */
  struct ListPathsOptions final
  {
    /**
     * Valid only when Hierarchical Namespace is enabled for the account. If "true", the user
     * identity values returned in the owner and group fields of each list entry will be transformed
     * from Azure Active Directory Object IDs to User Principal Names. If "false", the values will
     * be returned as Azure Active Directory Object IDs. The default value is false. Note that group
     * and application Object IDs are not translated because they do not have unique friendly names.
     */
    Azure::Nullable<bool> UserPrincipalName;

    /**
     * The number of paths returned with each invocation is limited. If the number of paths to be
     * returned exceeds this limit, a continuation token is returned in the response header
     * x-ms-continuation. When a continuation token is returned in the response, it must be
     * specified in a subsequent invocation of the list operation to continue listing the paths.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * An optional value that specifies the maximum number of items to return. If omitted or greater
     * than 5,000, the response will include up to 5,000 items.
     */
    Azure::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::FileSystemClient::GetAccessPolicy.
   */
  struct GetFileSystemAccessPolicyOptions final
  {
    /**
     * Optional conditions that must be met to perform this operation.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::FileSystemClient::SetAccessPolicy.
   */
  struct SetFileSystemAccessPolicyOptions final
  {
    /**
     * Specifies whether data in the file system may be accessed publicly and the level of access.
     */
    Models::PublicAccessType AccessType = Models::PublicAccessType::None;

    /**
     * Stored access policies that you can use to provide fine grained control over file system
     * permissions.
     */
    std::vector<Models::SignedIdentifier> SignedIdentifiers;

    /**
     * Optional conditions that must be met to perform this operation.
     */
    FileSystemAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::DataLakeFileSystemClient::RenameDirectory.
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct RenameDirectoryOptions final
  {
    /**
     * If not specified, the source's file system is used. Otherwise, rename to destination file
     * system.
     */
    Azure::Nullable<std::string> DestinationFileSystem;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    PathAccessConditions SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::Append.
   */
  struct AppendFileOptions final
  {
    /**
     * Specify the transactional hash for the body, to be validated by the service.
     */
    Azure::Nullable<Storage::ContentHash> TransactionalContentHash;

    /**
     * Specify the lease access conditions.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::Flush.
   */
  struct FlushFileOptions final
  {
    /**
     * If "true", uncommitted data is retained after the flush operation completes; otherwise, the
     * uncommitted data is deleted after the flush operation.  The default is false.  Data at
     * offsets less than the specified position are written to the file when flush succeeds, but
     * this optional parameter allows data after the flush position to be retained for a future
     * flush operation.
     */
    Azure::Nullable<bool> RetainUncommittedData;

    /**
     * Azure Storage Events allow applications to receive notifications when files change. When
     * Azure Storage Events are enabled, a file changed event is raised. This event has a property
     * indicating whether this is the final change to distinguish the difference between an
     * intermediate flush to a file stream and the final close of a file stream. The close query
     * parameter is valid only when the action is "flush" and change notifications are enabled. If
     * the value of close is "true" and the flush operation completes successfully, the service
     * raises a file change notification with a property indicating that this is the final update
     * (the file stream has been closed). If "false" a change notification is raised indicating the
     * file has changed. The default is false. This query parameter is set to true by the Hadoop
     * ABFS driver to indicate that the file stream has been closed."
     */
    Azure::Nullable<bool> Close;

    /**
     * The service stores this value and is returned for "Read & Get Properties" operations. If this
     * property is not specified on the request, then the property will be cleared for the file.
     * Subsequent calls to "Read & Get Properties" will not return this property unless it is
     * explicitly set on that file again.
     */
    Azure::Nullable<Storage::ContentHash> ContentHash;

    /**
     * Specify the HTTP headers for this path.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::PathClient::SetAccessControlList.
   */
  struct SetPathAccessControlListOptions final
  {
    /**
     * The owner of the path or directory.
     */
    Azure::Nullable<std::string> Owner;

    /**
     * The owning group of the path or directory.
     */
    Azure::Nullable<std::string> Group;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::SetPermissions.
   */
  struct SetPathPermissionsOptions final
  {
    /**
     * The owner of the path or directory.
     */
    Azure::Nullable<std::string> Owner;

    /**
     * The owning group of the path or directory.
     */
    Azure::Nullable<std::string> Group;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::SetHttpHeaders.
   */
  struct SetPathHttpHeadersOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::SetMetadata.
   */
  struct SetPathMetadataOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::Create.
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/rest/api/storageservices/datalakestoragegen2/path/create
   */
  struct CreatePathOptions final
  {
    /**
     * Specify the HTTP headers for this path.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * User-defined metadata to be stored with the path. Note that the string may only contain ASCII
     * characters in the ISO-8859-1 character set.  If the filesystem exists, any metadata not
     * included in the list will be removed.  All metadata are removed if the header is omitted.  To
     * merge new and existing metadata, first get all existing metadata and the current E-Tag, then
     * make a conditional request with the E-Tag and include values for all metadata.
     */
    Storage::Metadata Metadata;

    /**
     * Only valid if Hierarchical Namespace is enabled for the account. When creating a file or
     * directory and the parent folder does not have a default ACL, the umask restricts the
     * permissions of the file or directory to be created.  The resulting permission is given by p
     * bitwise and not u, where p is the permission and u is the umask.  For example, if p is 0777
     * and u is 0057, then the resulting permission is 0720.  The default permission is 0777 for a
     * directory and 0666 for a file. The default umask is 0027.  The umask must be specified in
     * 4-digit octal notation (e.g. 0766).
     */
    Azure::Nullable<std::string> Umask;

    /**
     * Only valid if Hierarchical Namespace is enabled for the account. Sets POSIX access
     * permissions for the file owner, the file owning group, and others. Each class may be granted
     * read, write, or execute permission. The sticky bit is also supported.  Both symbolic
     * (rwxrw-rw-) and 4-digit octal notation (e.g. 0766) are supported.
     */
    Azure::Nullable<std::string> Permissions;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::DirectoryClient::Delete.
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/rest/api/storageservices/datalakestoragegen2/path/delete
   */
  struct DeletePathOptions final
  {
    /**
     * Required and valid only when the resource is a directory. If "true", all paths beneath the
     * directory will be deleted. If "false" and the directory is non-empty, an error occurs.
     */
    Azure::Nullable<bool> Recursive;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::GetProperties.
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/rest/api/storageservices/datalakestoragegen2/path/getproperties
   */
  struct GetPathPropertiesOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::DataLake::PathClient::GetAccessControlList.
   */
  struct GetPathAccessControlListOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::PathClient::Read.
   * @remark Some optional parameter is mandatory in certain combination.
   *         More details:
   * https://docs.microsoft.com/rest/api/storageservices/datalakestoragegen2/path/read
   */
  struct DownloadFileOptions final
  {
    /**
     * Specify the range of the resource to be retrieved.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * The hash algorithm used to calculate the hash for the returned content.
     */
    Azure::Nullable<HashAlgorithm> RangeHashAlgorithm;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileClient::Create.
   */
  struct RenameFileOptions final
  {
    /**
     * If not specified, the source's file system is used. Otherwise, rename to destination file
     * system.
     */
    Azure::Nullable<std::string> DestinationFileSystem;

    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    PathAccessConditions SourceAccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileClient::Delete.
   */
  struct DeleteFileOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  using RenameSubdirectoryOptions = RenameDirectoryOptions;

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::DirectoryClient::Delete.
   */
  struct DeleteDirectoryOptions final
  {
    /**
     * Specify the access condition for the path.
     */
    PathAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for DirectoryClient::SetAccessControlListRecursive.
   */
  struct SetPathAccessControlListRecursiveOptions final
  {
    /**
     * When performing setAccessControlRecursive on a directory, the number of paths that are
     * processed with each invocation is limited.  If the number of paths to be processed exceeds
     * this limit, a continuation token is returned in this response header.  When a continuation
     * token is returned in the response, it must be specified in a subsequent invocation of the
     * setAccessControlRecursive operation to continue the setAccessControlRecursive operation on
     * the directory.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * It specifies the maximum number of files or directories on which the acl change will be
     * applied. If omitted or greater than 2,000, the request will process up to 2,000 items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Optional. If set to false, the operation will terminate quickly on encountering user
     * errors (4XX). If true, the operation will ignore user errors and proceed with the operation
     * on other sub-entities of the directory. Continuation token will only be returned when
     * ContinueOnFailure is true in case of user errors. If not set the default value is false for
     * this.
     */
    Azure::Nullable<bool> ContinueOnFailure;
  };

  using UpdatePathAccessControlListRecursiveOptions = SetPathAccessControlListRecursiveOptions;

  using RemovePathAccessControlListRecursiveOptions = SetPathAccessControlListRecursiveOptions;

  using CreateFileOptions = CreatePathOptions;
  using CreateDirectoryOptions = CreatePathOptions;

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileClient::UploadFrom.
   */
  struct UploadFileFromOptions final
  {
    /**
     * The standard HTTP header system properties to set.
     */
    Models::PathHttpHeaders HttpHeaders;

    /**
     * Name-value pairs associated with the blob as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * Options for parallel transfer.
     */
    struct
    {
      /**
       * File smaller than this will be uploaded with a single upload operation. This value
       * cannot be larger than 5000 MiB.
       */
      int64_t SingleUploadThreshold = 256 * 1024 * 1024;

      /**
       * The maximum number of bytes in a single request. This value cannot be larger than
       * 4000 MiB.
       */
      Azure::Nullable<int64_t> ChunkSize;

      /**
       * The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  using ScheduleFileExpiryOriginType = Blobs::Models::ScheduleBlobExpiryOriginType;

  /**
   * @brief Optional parameters for #Azure::Storage::Files::DataLake::FileClient::UploadFrom.
   */
  struct ScheduleFileDeletionOptions final
  {
    /**
     * The expiry time from the specified origin. Only work if ExpiryOrigin is
     * ScheduleFileExpiryOriginType::RelativeToCreation or
     * ScheduleFileExpiryOriginType::RelativeToNow.
     */
    Azure::Nullable<std::chrono::milliseconds> TimeToExpire;

    /**
     * The expiry time in RFC1123 format. Only work if ExpiryOrigin is
     * ScheduleFileExpiryOriginType::Absolute.
     */
    Azure::Nullable<DateTime> ExpiresOn;
  };

  using AcquireLeaseOptions = Blobs::AcquireLeaseOptions;
  using BreakLeaseOptions = Blobs::BreakLeaseOptions;
  using RenewLeaseOptions = Blobs::RenewLeaseOptions;
  using ReleaseLeaseOptions = Blobs::ReleaseLeaseOptions;
  using ChangeLeaseOptions = Blobs::ChangeLeaseOptions;

}}}} // namespace Azure::Storage::Files::DataLake

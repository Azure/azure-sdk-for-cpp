// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <azure/core/internal/client_options.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/common/access_conditions.hpp>

#include "azure/storage/files/shares/rest_client.hpp"

/* cSpell:ignore dacl */

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  /**
   * @brief Client options used to initialize share clients.
   */
  struct ShareClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * API version used by this client.
     */
    std::string ApiVersion;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::ListShares.
   */
  struct ListSharesOptions final
  {
    /**
     * Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Include this parameter to specify one or more datasets to include in the response.
     */
    Azure::Nullable<Models::ListSharesIncludeFlags> ListSharesIncludeFlags;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::SetProperties.
   */
  struct SetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareServiceClient::GetProperties.
   */
  struct GetServicePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::Create.
   */
  struct CreateShareOptions final
  {
    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * Specifies the access tier of the share. This is only valid for standard file account
     * and the value can only be one of `Hot`, `Cool` or `TransactionOptimized`
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * Specifies the maximum size of the share, in gigabytes.
     */
    Azure::Nullable<int64_t> ShareQuotaInGiB;

    /**
     * Specifies the enabled protocols on the share. If they're not specified, the default is SMB.
     */
    Azure::Nullable<Models::ShareProtocols> EnabledProtocols;

    /**
     * Specifies the root squashing behavior on the share when NFS is enabled. If it's not
     * specified, the default is NoRootSquash.
     */
    Azure::Nullable<Models::ShareRootSquash> RootSquash;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::Delete.
   */
  struct DeleteShareOptions final
  {
    /**
     * Specifies the option include to delete the base share and all of its snapshots.
     */
    Azure::Nullable<bool> DeleteSnapshots;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::CreateSnapshot.
   */
  struct CreateShareSnapshotOptions final
  {
    /**
     * The metadata to be set on the snapshot of the share.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetProperties.
   */
  struct GetSharePropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetProperties.
   */
  struct SetSharePropertiesOptions final
  {
    /**
     * Specifies the access tier of the share. This is only valid for standard file account
     * and the value can only be one of `Hot`, `Cool` or `TransactionOptimized`
     */
    Azure::Nullable<Models::AccessTier> AccessTier;

    /**
     * Specifies the maximum size of the share, in gigabytes.
     */
    Azure::Nullable<int64_t> ShareQuotaInGiB;

    /**
     * Specifies the root squashing behavior on the share when NFS is enabled. If it's not
     * specified, the default is NoRootSquash.
     */
    Azure::Nullable<Models::ShareRootSquash> RootSquash;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetMetadata.
   */
  struct SetShareMetadataOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetAccessPolicy.
   */
  struct GetShareAccessPolicyOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::SetAccessPolicy.
   */
  struct SetShareAccessPolicyOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetStatistics.
   */
  struct GetShareStatisticsOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::CreatePermission.
   */
  struct CreateSharePermissionOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareClient::GetPermission.
   */
  struct GetSharePermissionOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareDirectoryClient::Create.
   */
  struct CreateDirectoryOptions final
  {
    /**
     * A name-value pair to associate with a directory object.
     */
    Storage::Metadata Metadata;

    /**
     * This permission is the security descriptor for the directory specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> DirectoryPermission;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::RenameFile.
   */
  struct RenameFileOptions final
  {
    /**
     * A boolean value for if the destination file already exists, whether this request will
     * overwrite the file or not. If true, the rename will succeed and will overwrite the
     * destination file. If not provided or if false and the destination file does exist, the
     * request will not overwrite the destination file. If provided and the destination file doesn't
     * exist, the rename will succeed.
     */
    Azure::Nullable<bool> ReplaceIfExists;

    /**
     * A boolean value that specifies whether the ReadOnly attribute on a preexisting destination
     * file should be respected. If true, the rename will succeed, otherwise, a previous file at the
     * destination with the ReadOnly attribute set will cause the rename to fail. ReplaceIfExists
     * must also be true.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    LeaseAccessConditions SourceAccessConditions;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used.A value of preserve may be passed to keep an existing value unchanged.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::RenameSubdirectory.
   */
  struct RenameDirectoryOptions final
  {
    /**
     * A boolean value for if the destination directory already exists, whether this request will
     * overwrite the file or not. If true, the rename will succeed and will overwrite the
     * destination directory. If not provided or if false and the destination directory does exist,
     * the request will not overwrite the destination directory. If provided and the destination
     * file doesn't exist, the rename will succeed.
     */
    Azure::Nullable<bool> ReplaceIfExists;

    /**
     * A boolean value that specifies whether the ReadOnly attribute on a preexisting destination
     * directory should be respected. If true, the rename will succeed, otherwise, a previous file
     * at the destination with the ReadOnly attribute set will cause the rename to fail.
     * ReplaceIfExists must also be true.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specify the access condition for the path.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * The access condition for source path.
     */
    LeaseAccessConditions SourceAccessConditions;

    /**
     * SMB properties to set for the directory.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used.A value of preserve may be passed to keep an existing value unchanged.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareDirectoryClient::Delete.
   */
  struct DeleteDirectoryOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::GetProperties.
   */
  struct GetDirectoryPropertiesOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::SetProperties.
   */
  struct SetDirectoryPropertiesOptions final
  {
    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Nullable<std::string> FilePermission;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::SetMetadata.
   */
  struct SetDirectoryMetadataOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ListFilesAndDirectories.
   */
  struct ListFilesAndDirectoriesOptions final
  {
    /**
     * Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Nullable<std::string> Prefix;

    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Include this parameter to specify one or more datasets to include in the response.
     */
    Models::ListFilesIncludeFlags Include = Models::ListFilesIncludeFlags ::None;

    /**
     * This header is implicitly assumed to be true if include query parameter is not empty. If
     * true, the Content-Length property will be up to date.
     */
    Nullable<bool> IncludeExtendedInfo;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ListHandles.
   */
  struct ListDirectoryHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to
     * request the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;

    /**
     * Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Nullable<bool> Recursive;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ForceCloseHandle.
   */
  struct ForceCloseDirectoryHandleOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ForceCloseAllHandles.
   */
  struct ForceCloseAllDirectoryHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Nullable<bool> Recursive;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Create.
   */
  struct CreateFileOptions final
  {
    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * SMB properties to set for the file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * Specifies the HttpHeaders of the file.
     */
    Models::FileHttpHeaders HttpHeaders;

    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Delete.
   */
  struct DeleteFileOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::Download.
   */
  struct DownloadFileOptions final
  {
    /**
     * Downloads only the bytes of the file from this range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * When specified together with Range, service returns hash for the range as long as the
     * range is less than or equal to 4 MiB in size. Only MD5 is supported for now.
     */
    Azure::Nullable<HashAlgorithm> RangeHashAlgorithm;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::StartCopy.
   */
  struct StartFileCopyOptions final
  {
    /**
     * A name-value pair to associate with a file storage object.
     */
    Storage::Metadata Metadata;

    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * SMB properties to set for the destination file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * Specifies the option to copy file security descriptor from source file or to set it
     * using the value which is defined by the SMB properties.
     */
    Azure::Nullable<Models::PermissionCopyMode> PermissionCopyMode;

    /**
     * Specifies the option to overwrite the target file if it already exists and has
     * read-only attribute set.
     */
    Azure::Nullable<bool> IgnoreReadOnly;

    /**
     * Specifies the option to set archive attribute on a target file. True means archive
     * attribute will be set on a target file despite attribute overrides or a source file state.
     */
    Azure::Nullable<bool> SetArchiveAttribute;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::AbortCopy.
   */
  struct AbortFileCopyOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::GetProperties.
   */
  struct GetFilePropertiesOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::SetProperties.
   */
  struct SetFilePropertiesOptions final
  {
    /**
     * This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Nullable<std::string> Permission;

    /**
     * Specify this to resize a file to the specified value.
     */
    Azure::Nullable<int64_t> Size;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::SetMetadata.
   */
  struct SetFileMetadataOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::UploadRange.
   */
  struct UploadFileRangeOptions final
  {
    /**
     * An MD5 hash of the content. This hash is used to verify the integrity of the data
     * during transport. When the TransactionalContentHash parameter is specified, the File service
     * compares the hash of the content that has arrived with the header value that was sent. If the
     * two hashes do not match, the operation will fail with error code 400 (Bad Request).
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::ClearRange.
   */
  struct ClearFileRangeOptions final
  {
    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::UploadRangeFromUri.
   */
  struct UploadFileRangeFromUriOptions final
  {
    /**
     * Specify the hash calculated for the range of bytes that must be read from the copy
     * source.
     */
    Azure::Nullable<ContentHash> TransactionalContentHash;

    /**
     * Specify the access condition for the source. Only ContentHash with Crc64 is supported.
     */
    ContentHashAccessConditions SourceAccessCondition;

    /**
     * The operation will only succeed if the lease access condition is met.
     */
    LeaseAccessConditions AccessConditions;

    /**
     * Specifies if the file last write time should be set to the current time,
     * or the last write time currently associated with the file should be preserved.
     */
    Azure::Nullable<Models::FileLastWrittenMode> FileLastWrittenMode;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::GetRangeList.
   */
  struct GetFileRangeListOptions final
  {
    /**
     * The range to be get from service.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::ListHandles.
   */
  struct ListFileHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;

    /**
     * Specifies the maximum number of entries to return. If the request does not specify
     * PageSizeHint, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Nullable<int32_t> PageSizeHint;
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::ForceCloseHandle.
   */
  struct ForceCloseFileHandleOptions final
  {
  };

  /**
   * @brief Optional parameters for
   * #Azure::Storage::Files::Shares::ShareFileClient::ForceCloseAllHandles.
   */
  struct ForceCloseAllFileHandlesOptions final
  {
    /**
     * A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Nullable<std::string> ContinuationToken;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::DownloadTo.
   */
  struct DownloadFileToOptions final
  {
    /**
     * Downloads only the bytes of the file from this range.
     */
    Azure::Nullable<Core::Http::HttpRange> Range;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * The size of the first range request in bytes. Files smaller than this limit will be
       * downloaded in a single request. Files larger than this limit will continue being downloaded
       * in chunks of size ChunkSize.
       */
      int64_t InitialChunkSize = 256 * 1024 * 1024;

      /**
       * The maximum number of bytes in a single request.
       */
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareFileClient::UploadFrom.
   */
  struct UploadFileFromOptions final
  {
    /**
     * The standard HTTP header system properties to set.
     */
    Models::FileHttpHeaders HttpHeaders;

    /**
     * Name-value pairs associated with the file as metadata.
     */
    Storage::Metadata Metadata;

    /**
     * SMB properties to set for the destination file.
     */
    Models::FileSmbProperties SmbProperties;

    /**
     * If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.PermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Nullable<std::string> FilePermission;

    /**
     * @brief Options for parallel transfer.
     */
    struct
    {
      /**
       * File smaller than this will be uploaded with a single upload operation. This value
       * cannot be larger than 4 MiB.
       */
      int64_t SingleUploadThreshold = 4 * 1024 * 1024;

      /**
       * The maximum number of bytes in a single request.
       */
      int64_t ChunkSize = 4 * 1024 * 1024;

      /**
       * The maximum number of threads that may be used in a parallel transfer.
       */
      int32_t Concurrency = 5;
    } TransferOptions;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Acquire.
   */
  struct AcquireLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Change.
   */
  struct ChangeLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Release.
   */
  struct ReleaseLeaseOptions final
  {
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Break.
   */
  struct BreakLeaseOptions final
  {
    /**
     * Proposed duration the lease should continue before it is broken, in seconds,
     * between 0 and 60. This break period is only used if it is shorter than the time remaining on
     * the lease. If longer, the time remaining on the lease is used. A new lease will not be
     * available before the break period has expired, but the lease may be held for longer than the
     * break period.
     */
    Azure::Nullable<int32_t> BreakPeriod;
  };

  /**
   * @brief Optional parameters for #Azure::Storage::Files::Shares::ShareLeaseClient::Renew.
   */
  struct RenewLeaseOptions final
  {
  };
}}}} // namespace Azure::Storage::Files::Shares
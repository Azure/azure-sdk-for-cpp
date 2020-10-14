// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/nullable.hpp"
#include "azure/storage/common/access_conditions.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  /**
   * @brief Service client options used to initalize ServiceClient.
   */
  struct ServiceClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryOptions RetryOptions;
  };

  /**
   * @brief Share client options used to initalize ShareClient.
   */
  struct ShareClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryOptions RetryOptions;
  };

  /**
   * @brief Directory client options used to initalize DirectoryClient.
   */
  struct DirectoryClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryOptions RetryOptions;
  };

  /**
   * @brief File client options used to initalize FileClient.
   */
  struct FileClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    StorageRetryOptions RetryOptions;
  };

  struct ListSharesSegmentOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief  A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of entries to return. If the request does not specify
     * maxresults, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Include this parameter to specify one or more datasets to include in the response.
     */
    Azure::Core::Nullable<ListSharesIncludeType> ListSharesInclude;
  };

  struct SetServicePropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct GetServicePropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct CreateShareOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A name-value pair to associate with a file storage object.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief Specifies the maximum size of the share, in gigabytes.
     */
    Azure::Core::Nullable<int32_t> ShareQuota;
  };

  struct DeleteShareOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Specifies the option include to delete the base share and all of its snapshots.
     */
    Azure::Core::Nullable<bool> IncludeSnapshots;
  };

  struct CreateShareSnapshotOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The metadata to be set on the snapshot of the share.
     */
    std::map<std::string, std::string> Metadata;
  };

  struct GetSharePropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct SetShareQuotaOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct SetShareMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct GetShareAccessPolicyOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct SetShareAccessPolicyOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct GetShareStatsOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct CreateSharePermissionOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct GetSharePermissionOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for ShareClient::AcquireLease.
   */
  struct AcquireShareLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for ShareClient::ChangeLease.
   */
  struct ChangeShareLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for ShareClient::ReleaseLease.
   */
  struct ReleaseShareLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for ShareClient::BreakLease.
   */
  struct BreakShareLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Proposed duration the lease should continue before it is broken, in seconds,
     * between 0 and 60. This break period is only used if it is shorter than the time remaining on
     * the lease. If longer, the time remaining on the lease is used. A new lease will not be
     * available before the break period has expired, but the lease may be held for longer than the
     * break period.
     */
    Azure::Core::Nullable<int32_t> BreakPeriod;
  };

  /**
   * @brief Optional parameters for ShareClient::BreakLease.
   */
  struct RenewShareLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct CreateDirectoryOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A name-value pair to associate with a directory object.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief This permission is the security descriptor for the directory specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Core::Nullable<std::string> DirectoryPermission;

    /**
     * @brief SMB properties to set for the directory.
     */
    FileShareSmbProperties SmbProperties;
  };

  struct DeleteDirectoryOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct GetDirectoryPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct SetDirectoryPropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.FilePermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Core::Nullable<std::string> FilePermission;
  };

  struct SetDirectoryMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct ListFilesAndDirectoriesSegmentOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of entries to return. If the request does not specify
     * maxresults, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;
  };

  struct ListDirectoryHandlesSegmentOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of entries to return. If the request does not specify
     * maxresults, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Core::Nullable<bool> Recursive;
  };

  struct ForceCloseDirectoryHandleOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct ForceCloseAllDirectoryHandlesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies operation should apply to the directory specified in the URI, its files, its
     * subdirectories and their files.
     */
    Azure::Core::Nullable<bool> Recursive;
  };

  struct CreateFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Core::Nullable<std::string> Permission;

    /**
     * @brief SMB properties to set for the file.
     */
    FileShareSmbProperties SmbProperties;

    /**
     * @brief Specifies the HttpHeaders of the file.
     */
    FileShareHttpHeaders HttpHeaders;

    /**
     * @brief A name-value pair to associate with a file storage object.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct DeleteFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct DownloadFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Downloads only the bytes of the file from this offset.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief Returns at most this number of bytes of the file from the offset. Null means
     * download until the end.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief When this parameter is set to true and specified together with the Range parameter,
     * the service returns the MD5 hash for the range, as long as the range is less than or equal to
     * 4 MB in size.
     */
    Azure::Core::Nullable<bool> GetRangeContentMd5;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct StartCopyFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A name-value pair to associate with a file storage object.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Core::Nullable<std::string> Permission;

    /**
     * @brief SMB properties to set for the destination file.
     */
    FileShareSmbProperties SmbProperties;

    /**
     * @brief Specifies the option to copy file security descriptor from source file or to set it
     * using the value which is defined by the smb properties.
     */
    Azure::Core::Nullable<PermissionCopyModeType> PermissionCopyMode;

    /**
     * @brief Specifies the option to overwrite the target file if it already exists and has
     * read-only attribute set.
     */
    Azure::Core::Nullable<bool> IgnoreReadOnly;

    /**
     * @brief Specifies the option to set archive attribute on a target file. True means archive
     * attribute will be set on a target file despite attribute overrides or a source file state.
     */
    Azure::Core::Nullable<bool> SetArchiveAttribute;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct AbortCopyFileOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct GetFilePropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct SetFilePropertiesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief This permission is the security descriptor for the file specified in the Security
     * Descriptor Definition Language (SDDL). If not specified, 'inherit' is used.
     */
    Azure::Core::Nullable<std::string> Permission;

    /**
     * @brief Specify this to resize a file to the specified value.
     */
    Azure::Core::Nullable<int64_t> Size;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct SetFileMetadataOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct UploadFileRangeOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief An MD5 hash of the content. This hash is used to verify the integrity of the data
     * during transport. When the ContentMD5 parameter is specified, the File service compares the
     * hash of the content that has arrived with the header value that was sent. If the two hashes
     * do not match, the operation will fail with error code 400 (Bad Request).
     */
    Azure::Core::Nullable<std::string> TransactionalMd5;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct ClearFileRangeOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct UploadFileRangeFromUrlOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The offset of the source file.
     */
    Azure::Core::Nullable<int64_t> SourceOffset;

    /**
     * @brief The length of the source file.
     */
    Azure::Core::Nullable<int64_t> SourceLength;

    /**
     * @brief Specify the crc64 calculated for the range of bytes that must be read from the copy
     * source.
     */
    Azure::Core::Nullable<std::string> SourceContentCrc64;

    /**
     * @brief Specify the crc64 value to operate only on range with a matching crc64 checksum.
     */
    Azure::Core::Nullable<std::string> SourceIfMatchCrc64;

    /**
     * @brief Specify the crc64 value to operate only on range without a matching crc64 checksum.
     */
    Azure::Core::Nullable<std::string> SourceIfNoneMatchCrc64;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct GetFileRangeListOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The offset of the ranges to be get from service.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief The length starting from the offset to be get from the service. When present, 'Offset'
     * must not be null, otherwise it is ignored.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief The previous snapshot parameter is an opaque DateTime value that, when present,
     * specifies the previous snapshot.
     */
    Azure::Core::Nullable<std::string> PrevShareSnapshot;

    /**
     * @brief The operation will only succeed if the access condition is met.
     */
    LeaseAccessConditions AccessConditions;
  };

  struct ListFileHandlesSegmentOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;

    /**
     * @brief Specifies the maximum number of entries to return. If the request does not specify
     * maxresults, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;
  };

  struct ForceCloseFileHandleOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  struct ForceCloseAllFileHandlesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief A string value that identifies the portion of the list to be returned with the next
     * close operation. The operation returns a marker value within the response body if the force
     * close was not complete. The marker value may then be used in a subsequent call to
     * close the next handle. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> ContinuationToken;
  };

  /**
   * @brief Optional parameters for FileClient::DownloadTo.
   */
  struct DownloadFileToOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Downloads only the bytes of the file from this offset.
     */
    Azure::Core::Nullable<int64_t> Offset;

    /**
     * @brief Returns at most this number of bytes of the file from the offset. Null means
     * download until the end.
     */
    Azure::Core::Nullable<int64_t> Length;

    /**
     * @brief The size of the first range request in bytes. Files smaller than this limit will be
     * downloaded in a single request. Files larger than this limit will continue being downloaded
     * in chunks of size ChunkSize.
     */
    Azure::Core::Nullable<int64_t> InitialChunkSize;

    /**
     * @brief The maximum number of bytes in a single request.
     */
    Azure::Core::Nullable<int64_t> ChunkSize;

    /**
     * @brief The maximum number of threads that may be used in a parallel transfer.
     */
    int Concurrency = 1;
  };

  /**
   * @brief Optional parameters for FileClient::AcquireLease.
   */
  struct AcquireFileLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for FileClient::ChangeLease.
   */
  struct ChangeFileLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for FileClient::ReleaseLease.
   */
  struct ReleaseFileLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for FileClient::BreakLease.
   */
  struct BreakFileLeaseOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };

  /**
   * @brief Optional parameters for FileClient::UploadFrom.
   */
  struct UploadFileFromOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief The standard HTTP header system properties to set.
     */
    FileShareHttpHeaders HttpHeaders;

    /**
     * @brief Name-value pairs associated with the file as metadata.
     */
    std::map<std::string, std::string> Metadata;

    /**
     * @brief The maximum number of bytes in a single request.
     */
    Azure::Core::Nullable<int64_t> ChunkSize;

    /**
     * @brief SMB properties to set for the destination file.
     */
    FileShareSmbProperties SmbProperties;

    /**
     * @brief If specified the permission (security descriptor) shall be set for the directory.
     * This option can be used if Permission size is <= 8KB, else SmbProperties.FilePermissionKey
     * shall be used. Default value: 'inherit'. If SDDL is specified as input, it must have owner,
     * group and dacl.
     */
    Azure::Core::Nullable<std::string> FilePermission;

    /**
     * @brief The maximum number of threads that may be used in a parallel transfer.
     */
    int Concurrency = 1;
  };
}}}} // namespace Azure::Storage::Files::Shares

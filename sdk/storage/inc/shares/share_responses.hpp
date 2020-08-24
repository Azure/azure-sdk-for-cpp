// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "protocol/share_rest_client.hpp"
#include "share_constants.hpp"
#include "share_file_attribute.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  // ServiceClient models:

  using ListSharesSegmentResult = ServiceListSharesSegmentResult;
  using SetServicePropertiesResult = ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  using CreateShareResult = ShareCreateResult;
  using DeleteShareResult = ShareDeleteResult;
  using CreateShareSnapshotResult = ShareCreateSnapshotResult;
  using GetSharePropertiesResult = ShareGetPropertiesResult;
  using SetShareQuotaResult = ShareSetQuotaResult;
  using SetShareMetadataResult = ShareSetMetadataResult;
  using SetShareAccessPolicyResult = ShareSetAccessPolicyResult;
  using GetShareStatisticsResult = ShareGetStatisticsResult;
  using CreateSharePermissionResult = ShareCreatePermissionResult;
  using GetShareAccessPolicyResult = ShareGetAccessPolicyResult;
  using GetSharePermissionResult = ShareGetPermissionResult;

  // DirectoryClient models:
  using CreateDirectoryResult = DirectoryCreateResult;
  using DeleteDirectoryResult = DirectoryDeleteResult;
  using GetDirectoryPropertiesResult = DirectoryGetPropertiesResult;
  using SetDirectoryPropertiesResult = DirectorySetPropertiesResult;
  using SetDirectoryMetadataResult = DirectorySetMetadataResult;
  using ForceCloseDirectoryHandlesResult = DirectoryForceCloseHandlesResult;

  struct ListFilesAndDirectoriesSegmentedResult
  {
    std::string ServiceEndpoint;
    std::string ShareName;
    std::string ShareSnapshot;
    std::string DirectoryPath;
    std::string Prefix;
    std::string Marker;
    int32_t MaxResults = int32_t();
    std::string NextMarker;
    std::vector<DirectoryItem> DirectoryItems;
    std::vector<FileItem> FileItems;
  };

  struct ListDirectoryHandlesSegmentedResult
  {
    std::vector<HandleItem> HandleList;
    std::string NextMarker;
  };

  struct FileShareSmbProperties
  {
    /**
     * @brief Key of the permission to be set for the directory/file.
     */
    Azure::Core::Nullable<std::string> FilePermissionKey;

    /**
     * @brief If specified, the provided file attributes shall be set. Default value:
     * FileAttribute::Archive for file and FileAttribute::Directory for directory.
     * FileAttribute::None can also be specified as default.
     */
    FileAttributes Attributes = static_cast<FileAttributes>(0);

    /**
     * @brief Creation time for the file/directory. Default value: Now.
     */
    Azure::Core::Nullable<std::string> FileCreationTime = c_FileDefaultTimeValue;

    /**
     * @brief Last write time for the file/directory. Default value: Now.
     */

    Azure::Core::Nullable<std::string> FileLastWriteTime = c_FileDefaultTimeValue;
  };

  // FileClient models:
  using CreateFileResult = FileCreateResult;
  using DeleteFileResult = FileDeleteResult;
  using DownloadFileResult = FileDownloadResult;
  using StartCopyFileResult = FileStartCopyResult;
  using AbortCopyFileResult = FileAbortCopyResult;
  using GetFilePropertiesResult = FileGetPropertiesResult;
  using SetFilePropertiesResult = FileSetHTTPHeadersResult;
  using ResizeFileResult = FileSetHTTPHeadersResult;
  using SetFileMetadataResult = FileSetMetadataResult;
  using UploadFileRangeResult = FileUploadRangeResult;
  using ClearFileRangeResult = FileUploadRangeResult;
  using UploadFileRangeFromUrlResult = FileUploadRangeFromURLResult;
  using GetFileRangeListResult = FileGetRangeListResult;
  using ListFileHandlesSegmentedResult = ListDirectoryHandlesSegmentedResult;
  using ForceCloseFileHandlesResult = FileForceCloseHandlesResult;
  using AcquireFileLeaseResult = FileAcquireLeaseResult;
  using ReleaseFileLeaseResult = FileReleaseLeaseResult;
  using BreakFileLeaseResult = FileBreakLeaseResult;
  using ChangeFileLeaseResult = FileChangeLeaseResult;

  struct DownloadFileToResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t ContentLength = 0;
    FileShareHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    Azure::Core::Nullable<bool> IsServerEncrypted;
  };

  struct UploadFileFromResult
  {
    Azure::Core::Nullable<bool> IsServerEncrypted;
  };

}}}} // namespace Azure::Storage::Files::Shares

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_file_attribute.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  struct FileShareSmbProperties
  {
    /**
     * @brief Key of the permission to be set for the directory/file.
     */
    Azure::Core::Nullable<std::string> PermissionKey;

    /**
     * @brief If specified, the provided file attributes shall be set. Default value:
     * FileAttribute::Archive for file and FileAttribute::Directory for directory.
     * FileAttribute::None can also be specified as default.
     */
    FileAttributes Attributes = static_cast<FileAttributes>(0);

    /**
     * @brief Creation time for the file/directory..
     */
    Azure::Core::Nullable<Core::DateTime> CreatedOn;

    /**
     * @brief Last write time for the file/directory..
     */
    Azure::Core::Nullable<Core::DateTime> LastWrittenOn;
  };

  // ServiceClient models:

  using ListSharesSinglePageResult = ServiceListSharesSinglePageResult;
  using SetServicePropertiesResult = ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  struct CreateShareResult
  {
    bool Created = bool();
    std::string ETag;
    Core::DateTime LastModified;
  };
  
  struct DeleteShareResult
  {
    bool Deleted = bool();
  };
  using CreateShareSnapshotResult = ShareCreateSnapshotResult;
  using GetSharePropertiesResult = ShareGetPropertiesResult;
  using SetShareQuotaResult = ShareSetQuotaResult;
  using SetShareMetadataResult = ShareSetMetadataResult;
  using SetShareAccessPolicyResult = ShareSetAccessPolicyResult;
  using GetShareStatisticsResult = ShareGetStatisticsResult;
  using CreateSharePermissionResult = ShareCreatePermissionResult;
  using GetShareAccessPolicyResult = ShareGetAccessPolicyResult;
  using GetSharePermissionResult = ShareGetPermissionResult;
  using AcquireShareLeaseResult = ShareAcquireLeaseResult;
  using RenewShareLeaseResult = ShareRenewLeaseResult;
  using ReleaseShareLeaseResult = ShareReleaseLeaseResult;
  using BreakShareLeaseResult = ShareBreakLeaseResult;
  using ChangeShareLeaseResult = ShareChangeLeaseResult;

  // DirectoryClient models:
  struct CreateDirectoryResult
  {
    bool Created = bool();
    std::string ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    FileShareSmbProperties SmbProperties;
    Core::DateTime FileChangedOn;
    std::string FileId;
    std::string FileParentId;
  };

  struct DeleteDirectoryResult
  {
    bool Deleted = bool();
  };
  using GetDirectoryPropertiesResult = DirectoryGetPropertiesResult;
  using SetDirectoryPropertiesResult = DirectorySetPropertiesResult;
  using SetDirectoryMetadataResult = DirectorySetMetadataResult;
  using ForceCloseAllDirectoryHandlesResult = DirectoryForceCloseHandlesResult;

  struct ForceCloseDirectoryHandleResult
  {
  };

  struct ListFilesAndDirectoriesSinglePageResult
  {
    std::string ServiceEndpoint;
    std::string ShareName;
    std::string ShareSnapshot;
    std::string DirectoryPath;
    std::string Prefix;
    std::string PreviousContinuationToken;
    int32_t PageSizeHint = int32_t();
    std::string ContinuationToken;
    std::vector<DirectoryItem> DirectoryItems;
    std::vector<FileItem> FileItems;
  };

  struct ListDirectoryHandlesSinglePageResult
  {
    std::vector<HandleItem> Handles;
    std::string ContinuationToken;
  };

  // FileClient models:
  struct CreateFileResult
  {
    bool Created = bool();
    std::string ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    FileShareSmbProperties SmbProperties;
    Core::DateTime FileChangedOn;
    std::string FileId;
    std::string FileParentId;
  };

  struct DeleteFileResult
  {
    bool Deleted = bool();
  };
  using DownloadFileResult = FileDownloadResult;
  using StartCopyFileResult = FileStartCopyResult;
  using AbortCopyFileResult = FileAbortCopyResult;
  using GetFilePropertiesResult = FileGetPropertiesResult;
  using SetFilePropertiesResult = FileSetHttpHeadersResult;
  using ResizeFileResult = FileSetHttpHeadersResult;
  using SetFileMetadataResult = FileSetMetadataResult;
  using UploadFileRangeResult = FileUploadRangeResult;
  using ClearFileRangeResult = FileUploadRangeResult;
  using UploadFileRangeFromUrlResult = FileUploadRangeFromUrlResult;
  using GetFileRangeListResult = FileGetRangeListResult;
  using ListFileHandlesSinglePageResult = ListDirectoryHandlesSinglePageResult;
  using ForceCloseAllFileHandlesResult = FileForceCloseHandlesResult;
  using AcquireFileLeaseResult = FileAcquireLeaseResult;
  using ReleaseFileLeaseResult = FileReleaseLeaseResult;
  using BreakFileLeaseResult = FileBreakLeaseResult;
  using ChangeFileLeaseResult = FileChangeLeaseResult;

  struct DownloadFileToResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    int64_t ContentLength = 0;
    ShareFileHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    Azure::Core::Nullable<bool> IsServerEncrypted;
  };

  struct ForceCloseFileHandleResult
  {
  };

  struct UploadFileFromResult
  {
    Azure::Core::Nullable<bool> IsServerEncrypted;
  };

}}}}} // namespace Azure::Storage::Files::Shares::Models

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_file_attribute.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  // ServiceClient models:

  using ListSharesSinglePageResult = ServiceListSharesSinglePageResult;
  using SetServicePropertiesResult = ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  struct CreateShareResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
  };

  struct DeleteShareResult
  {
    bool Deleted = true;
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
  struct CreateShareDirectoryResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    std::string FilePermissionKey;
    std::string FileAttributes;
    Core::DateTime FileCreatedOn;
    Core::DateTime FileLastWrittenOn;
    Core::DateTime FileChangedOn;
    std::string FileId;
    std::string FileParentId;
  };

  struct DeleteShareDirectoryResult
  {
    bool Deleted = true;
  };

  using GetShareDirectoryPropertiesResult = DirectoryGetPropertiesResult;
  using SetShareDirectoryPropertiesResult = DirectorySetPropertiesResult;
  using SetShareDirectoryMetadataResult = DirectorySetMetadataResult;
  using ForceCloseAllShareDirectoryHandlesResult = DirectoryForceCloseHandlesResult;

  struct ForceCloseShareDirectoryHandleResult
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

  struct ListShareDirectoryHandlesSinglePageResult
  {
    std::vector<HandleItem> Handles;
    std::string ContinuationToken;
  };

  // FileClient models:
  struct CreateShareFileResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    std::string FilePermissionKey;
    std::string FileAttributes;
    Core::DateTime FileCreatedOn;
    Core::DateTime FileLastWrittenOn;
    Core::DateTime FileChangedOn;
    std::string FileId;
    std::string FileParentId;
  };

  struct DeleteShareFileResult
  {
    bool Deleted = true;
  };

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

  using DownloadShareFileResult = FileDownloadResult;
  using StartCopyShareFileResult = FileStartCopyResult;
  using AbortCopyShareFileResult = FileAbortCopyResult;
  using GetShareFilePropertiesResult = FileGetPropertiesResult;
  using SetShareFilePropertiesResult = FileSetHttpHeadersResult;
  using ResizeFileResult = FileSetHttpHeadersResult;
  using SetShareFileMetadataResult = FileSetMetadataResult;
  using UploadShareFileRangeResult = FileUploadRangeResult;
  using ClearShareFileRangeResult = FileUploadRangeResult;
  using UploadFileRangeFromUrlResult = FileUploadRangeFromUrlResult;
  using GetShareFileRangeListResult = FileGetRangeListResult;
  using ListShareFileHandlesSinglePageResult = ListShareDirectoryHandlesSinglePageResult;
  using ForceCloseAllShareFileHandlesResult = FileForceCloseHandlesResult;
  using AcquireShareFileLeaseResult = FileAcquireLeaseResult;
  using ReleaseShareFileLeaseResult = FileReleaseLeaseResult;
  using BreakShareFileLeaseResult = FileBreakLeaseResult;
  using ChangeShareFileLeaseResult = FileChangeLeaseResult;

  struct DownloadShareFileToResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    int64_t ContentLength = 0;
    ShareFileHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    bool IsServerEncrypted = false;
  };

  struct ForceCloseShareFileHandleResult
  {
  };

  struct UploadShareFileFromResult
  {
    bool IsServerEncrypted = false;
  };

}}}}} // namespace Azure::Storage::Files::Shares::Models

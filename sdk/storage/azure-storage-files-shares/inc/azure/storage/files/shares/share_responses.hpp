// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_file_attribute.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  // ServiceClient models:

  using ListSharesSinglePageResult = Details::ServiceListSharesSinglePageResult;
  using SetServicePropertiesResult = Details::ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  struct CreateShareResult
  {
    bool Created = true;
    std::string ETag;
    Core::DateTime LastModified;
    std::string RequestId;
  };

  struct DeleteShareResult
  {
    bool Deleted = true;
    std::string RequestId;
  };
  using CreateShareSnapshotResult = Details::ShareCreateSnapshotResult;
  using GetSharePropertiesResult = Details::ShareGetPropertiesResult;
  using SetShareQuotaResult = Details::ShareSetQuotaResult;
  using SetShareMetadataResult = Details::ShareSetMetadataResult;
  using SetShareAccessPolicyResult = Details::ShareSetAccessPolicyResult;
  using GetShareStatisticsResult = Details::ShareGetStatisticsResult;
  using CreateSharePermissionResult = Details::ShareCreatePermissionResult;
  using GetShareAccessPolicyResult = Details::ShareGetAccessPolicyResult;
  using GetSharePermissionResult = Details::ShareGetPermissionResult;
  using AcquireShareLeaseResult = Details::ShareAcquireLeaseResult;
  using RenewShareLeaseResult = Details::ShareRenewLeaseResult;
  using ReleaseShareLeaseResult = Details::ShareReleaseLeaseResult;
  using BreakShareLeaseResult = Details::ShareBreakLeaseResult;
  using ChangeShareLeaseResult = Details::ShareChangeLeaseResult;

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
    std::string RequestId;
  };

  struct DeleteShareDirectoryResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  using GetShareDirectoryPropertiesResult = Details::DirectoryGetPropertiesResult;
  using SetShareDirectoryPropertiesResult = Details::DirectorySetPropertiesResult;
  using SetShareDirectoryMetadataResult = Details::DirectorySetMetadataResult;
  using ForceCloseAllShareDirectoryHandlesResult = Details::DirectoryForceCloseHandlesResult;

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
    int32_t PageSizeHint = int32_t();
    std::string ContinuationToken;
    std::vector<DirectoryItem> DirectoryItems;
    std::vector<FileItem> FileItems;
    std::string RequestId;
  };

  struct ListShareDirectoryHandlesSinglePageResult
  {
    std::vector<HandleItem> Handles;
    std::string ContinuationToken;
    std::string RequestId;
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
    std::string RequestId;
  };

  struct DeleteShareFileResult
  {
    bool Deleted = true;
    std::string RequestId;
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

  using DownloadShareFileResult = Details::FileDownloadResult;
  using StartCopyShareFileResult = Details::FileStartCopyResult;
  using AbortCopyShareFileResult = Details::FileAbortCopyResult;
  using GetShareFilePropertiesResult = Details::FileGetPropertiesResult;
  using SetShareFilePropertiesResult = Details::FileSetHttpHeadersResult;
  using ResizeFileResult = Details::FileSetHttpHeadersResult;
  using SetShareFileMetadataResult = Details::FileSetMetadataResult;
  using UploadShareFileRangeResult = Details::FileUploadRangeResult;
  using ClearShareFileRangeResult = Details::FileUploadRangeResult;
  using UploadFileRangeFromUrlResult = Details::FileUploadRangeFromUrlResult;
  using GetShareFileRangeListResult = Details::FileGetRangeListResult;
  using ListShareFileHandlesSinglePageResult = ListShareDirectoryHandlesSinglePageResult;
  using ForceCloseAllShareFileHandlesResult = Details::FileForceCloseHandlesResult;

  struct DownloadShareFileToResult
  {
    std::string ETag;
    Core::DateTime LastModified;
    int64_t ContentLength = 0;
    ShareFileHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    bool IsServerEncrypted = false;
    std::string RequestId;
  };

  struct ForceCloseShareFileHandleResult
  {
    std::string RequestId;
  };

  struct UploadShareFileFromResult
  {
    bool IsServerEncrypted = false;
    std::string RequestId;
  };

}}}}} // namespace Azure::Storage::Files::Shares::Models

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_file_attributes.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  // ServiceClient models:

  using ListSharesSinglePageResult = Details::ServiceListSharesSinglePageResult;
  using SetServicePropertiesResult = Details::ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  struct CreateShareResult
  {
    bool Created = true;
    Azure::Core::ETag ETag;
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
  using SetSharePropertiesResult = Details::ShareSetPropertiesResult;
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
  struct FileSmbProperties
  {
    /**
     * @brief Permission key of the directory or file.
     */
    Azure::Core::Nullable<std::string> PermissionKey;

    /**
     * @brief If specified, the provided file attributes shall be set. Default value:
     * FileAttribute::Archive for file and FileAttribute::Directory for directory.
     * FileAttribute::None can also be specified as default.
     */
    FileAttributes Attributes;

    /**
     * @brief Creation time for the file/directory.
     */
    Azure::Core::Nullable<Core::DateTime> CreatedOn;

    /**
     * @brief Last write time for the file/directory.
     */
    Azure::Core::Nullable<Core::DateTime> LastWrittenOn;

    /**
     * @brief Changed time for the file/directory.
     */
    Azure::Core::Nullable<Core::DateTime> ChangedOn;

    /**
     * @brief The fileId of the file.
     */
    std::string FileId;

    /**
     * @brief The parentId of the file
     */
    std::string ParentFileId;
  };

  struct CreateShareDirectoryResult
  {
    bool Created = true;
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    FileSmbProperties SmbProperties;
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
    std::string RequestId;
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
    Azure::Core::ETag ETag;
    Core::DateTime LastModified;
    bool IsServerEncrypted = bool();
    FileSmbProperties SmbProperties;
    std::string RequestId;
  };

  struct DeleteShareFileResult
  {
    bool Deleted = true;
    std::string RequestId;
  };

  struct DownloadShareFileDetails
  {
    Core::DateTime LastModified;
    Storage::Metadata Metadata;
    Core::ETag ETag;
    Azure::Core::Nullable<Core::DateTime> CopyCompletedOn;
    Azure::Core::Nullable<std::string> CopyStatusDescription;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<CopyStatusType> CopyStatus;
    bool IsServerEncrypted = bool();
    FileSmbProperties SmbProperties;
    Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
    Azure::Core::Nullable<LeaseStateType> LeaseState;
    Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
  };

  struct DownloadShareFileResult
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> BodyStream;
    Azure::Core::Http::Range ContentRange;
    int64_t FileSize = 0;
    Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
    ShareFileHttpHeaders HttpHeaders;
    DownloadShareFileDetails Details;
    std::string RequestId;
  };

  using StartCopyShareFileResult = Details::FileStartCopyResult;
  using AbortCopyShareFileResult = Details::FileAbortCopyResult;
  struct GetShareFilePropertiesResult
  {
    Core::DateTime LastModified;
    Storage::Metadata Metadata;
    int64_t FileSize = int64_t();
    ShareFileHttpHeaders HttpHeaders;
    Core::ETag ETag;
    std::string RequestId;
    Azure::Core::Nullable<Core::DateTime> CopyCompletedOn;
    Azure::Core::Nullable<std::string> CopyStatusDescription;
    Azure::Core::Nullable<std::string> CopyId;
    Azure::Core::Nullable<std::string> CopyProgress;
    Azure::Core::Nullable<std::string> CopySource;
    Azure::Core::Nullable<CopyStatusType> CopyStatus;
    bool IsServerEncrypted = bool();
    FileSmbProperties SmbProperties;
    Azure::Core::Nullable<LeaseDurationType> LeaseDuration;
    Azure::Core::Nullable<LeaseStateType> LeaseState;
    Azure::Core::Nullable<LeaseStatusType> LeaseStatus;
  };
  using SetShareFilePropertiesResult = Details::FileSetHttpHeadersResult;
  using ResizeFileResult = Details::FileSetHttpHeadersResult;
  using SetShareFileMetadataResult = Details::FileSetMetadataResult;
  using UploadShareFileRangeResult = Details::FileUploadRangeResult;
  struct ClearShareFileRangeResult
  {
    Core::ETag ETag;
    Core::DateTime LastModified;
    std::string RequestId;
    bool IsServerEncrypted = bool();
  };
  using UploadFileRangeFromUriResult = Details::FileUploadRangeFromUrlResult;
  using GetShareFileRangeListResult = Details::FileGetRangeListResult;
  using ListShareFileHandlesSinglePageResult = ListShareDirectoryHandlesSinglePageResult;
  using ForceCloseAllShareFileHandlesResult = Details::FileForceCloseHandlesResult;

  struct DownloadShareFileToResult
  {
    int64_t FileSize = 0;
    Azure::Core::Http::Range ContentRange;
    ShareFileHttpHeaders HttpHeaders;
    DownloadShareFileDetails Details;
  };

  struct ForceCloseShareFileHandleResult
  {
    std::string RequestId;
  };

  struct UploadShareFileFromResult
  {
    bool IsServerEncrypted = false;
  };

}}}}} // namespace Azure::Storage::Files::Shares::Models

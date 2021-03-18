// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"

#include <azure/core/operation.hpp>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareFileClient;

  namespace Models {

    // ServiceClient models:

    using ListSharesSinglePageResult = _detail::ServiceListSharesSinglePageResult;
    using SetServicePropertiesResult = _detail::ServiceSetPropertiesResult;

    // ShareClient models:
    struct CreateShareResult
    {
      bool Created = true;
      Azure::ETag ETag;
      DateTime LastModified;
      std::string RequestId;
    };

    struct DeleteShareResult
    {
      bool Deleted = true;
      std::string RequestId;
    };
    using CreateShareSnapshotResult = _detail::ShareCreateSnapshotResult;
    using ShareProperties = _detail::ShareGetPropertiesResult;
    using SetSharePropertiesResult = _detail::ShareSetPropertiesResult;
    using SetShareMetadataResult = _detail::ShareSetMetadataResult;
    using GetShareAccessPolicyResult = _detail::ShareGetAccessPolicyResult;
    using SetShareAccessPolicyResult = _detail::ShareSetAccessPolicyResult;
    using ShareStatistics = _detail::ShareGetStatisticsResult;
    using CreateSharePermissionResult = _detail::ShareCreatePermissionResult;
    using AcquireShareLeaseResult = _detail::ShareAcquireLeaseResult;
    using RenewShareLeaseResult = _detail::ShareRenewLeaseResult;
    using ReleaseShareLeaseResult = _detail::ShareReleaseLeaseResult;
    using BreakShareLeaseResult = _detail::ShareBreakLeaseResult;
    using ChangeShareLeaseResult = _detail::ShareChangeLeaseResult;

    // DirectoryClient models:

    struct CreateDirectoryResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string RequestId;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      std::string ParentFileId;
      bool Created = false;
    };

    struct DeleteDirectoryResult
    {
      bool Deleted = true;
      std::string RequestId;
    };

    using ShareDirectoryProperties = _detail::DirectoryGetPropertiesResult;
    using SetDirectoryPropertiesResult = _detail::DirectorySetPropertiesResult;
    using SetDirectoryMetadataResult = _detail::DirectorySetMetadataResult;
    using ForceCloseAllDirectoryHandlesSinglePageResult = _detail::DirectoryForceCloseHandlesResult;

    struct ForceCloseDirectoryHandleResult
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
      Nullable<std::string> ContinuationToken;
      std::vector<DirectoryItem> DirectoryItems;
      std::vector<FileItem> FileItems;
      std::string RequestId;
    };

    struct ListDirectoryHandlesSinglePageResult
    {
      std::vector<HandleItem> Handles;
      Nullable<std::string> ContinuationToken;
      std::string RequestId;
    };

    // FileClient models:
    struct CreateFileResult
    {
      bool Created = true;
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      std::string RequestId;
    };

    struct DeleteFileResult
    {
      bool Deleted = true;
      std::string RequestId;
    };

    struct DownloadShareFileDetails
    {
      DateTime LastModified;
      Storage::Metadata Metadata;
      Azure::ETag ETag;
      Nullable<DateTime> CopyCompletedOn;
      Nullable<std::string> CopyStatusDescription;
      Nullable<std::string> CopyId;
      Nullable<std::string> CopyProgress;
      Nullable<std::string> CopySource;
      Nullable<CopyStatusType> CopyStatus;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      Nullable<LeaseDurationType> LeaseDuration;
      Nullable<LeaseStateType> LeaseState;
      Nullable<LeaseStatusType> LeaseStatus;
    };

    struct DownloadFileResult
    {
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t FileSize = 0;
      Nullable<Storage::ContentHash> TransactionalContentHash;
      FileHttpHeaders HttpHeaders;
      DownloadShareFileDetails Details;
      std::string RequestId;
    };

    using StartCopyFileResult = _detail::FileStartCopyResult;
    using AbortCopyFileResult = _detail::FileAbortCopyResult;
    using ShareFileProperties = _detail::FileGetPropertiesResult;
    using SetFilePropertiesResult = _detail::FileSetHttpHeadersResult;
    using ResizeFileResult = _detail::FileSetHttpHeadersResult;
    using SetFileMetadataResult = _detail::FileSetMetadataResult;
    using UploadFileRangeResult = _detail::FileUploadRangeResult;
    struct ClearFileRangeResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string RequestId;
      bool IsServerEncrypted = bool();
    };
    using UploadFileRangeFromUriResult = _detail::FileUploadRangeFromUrlResult;
    using GetFileRangeListResult = _detail::FileGetRangeListResult;
    using ListFileHandlesSinglePageResult = ListDirectoryHandlesSinglePageResult;
    using ForceCloseAllFileHandlesSinglePageResult = _detail::FileForceCloseHandlesResult;

    struct DownloadFileToResult
    {
      int64_t FileSize = 0;
      Azure::Core::Http::HttpRange ContentRange;
      FileHttpHeaders HttpHeaders;
      DownloadShareFileDetails Details;
    };

    struct ForceCloseFileHandleResult
    {
      std::string RequestId;
    };

    struct UploadFileFromResult
    {
      bool IsServerEncrypted = false;
    };

  } // namespace Models

  class StartCopyShareFileOperation : public Azure::Core::Operation<Models::ShareFileProperties> {
  public:
    std::string RequestId;
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    std::string CopyId;
    Models::CopyStatusType CopyStatus;
    Nullable<std::string> VersionId;

  public:
    Models::ShareFileProperties Value() const override { return m_pollResult; }

    StartCopyShareFileOperation() = default;

    StartCopyShareFileOperation(StartCopyShareFileOperation&&) = default;

    StartCopyShareFileOperation& operator=(StartCopyShareFileOperation&&) = default;

    ~StartCopyShareFileOperation() override {}

  private:
    std::string GetResumeToken() const override
    {
      // Not supported
      std::abort();
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override;

    Azure::Response<Models::ShareFileProperties> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    /**
     * @brief Get the raw HTTP response.
     * @return A pointer to #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    const Azure::Core::Http::RawResponse& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::shared_ptr<ShareFileClient> m_fileClient;
    Models::ShareFileProperties m_pollResult;

    friend class ShareFileClient;
  };
}}}} // namespace Azure::Storage::Files::Shares

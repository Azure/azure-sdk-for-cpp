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
    using GetServicePropertiesResult = FileServiceProperties;

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
    using GetSharePropertiesResult = _detail::ShareGetPropertiesResult;
    using SetSharePropertiesResult = _detail::ShareSetPropertiesResult;
    using SetShareMetadataResult = _detail::ShareSetMetadataResult;
    using SetShareAccessPolicyResult = _detail::ShareSetAccessPolicyResult;
    using GetShareStatisticsResult = _detail::ShareGetStatisticsResult;
    using CreateSharePermissionResult = _detail::ShareCreatePermissionResult;
    using GetShareAccessPolicyResult = _detail::ShareGetAccessPolicyResult;
    using GetSharePermissionResult = _detail::ShareGetPermissionResult;
    using AcquireShareLeaseResult = _detail::ShareAcquireLeaseResult;
    using RenewShareLeaseResult = _detail::ShareRenewLeaseResult;
    using ReleaseShareLeaseResult = _detail::ShareReleaseLeaseResult;
    using BreakShareLeaseResult = _detail::ShareBreakLeaseResult;
    using ChangeShareLeaseResult = _detail::ShareChangeLeaseResult;

    // DirectoryClient models:

    struct CreateShareDirectoryResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string RequestId;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      std::string ParentFileId;
      bool Created = false;
    };

    struct DeleteShareDirectoryResult
    {
      bool Deleted = true;
      std::string RequestId;
    };

    using GetShareDirectoryPropertiesResult = _detail::DirectoryGetPropertiesResult;
    using SetShareDirectoryPropertiesResult = _detail::DirectorySetPropertiesResult;
    using SetShareDirectoryMetadataResult = _detail::DirectorySetMetadataResult;
    using ForceCloseAllShareDirectoryHandlesSinglePageResult
        = _detail::DirectoryForceCloseHandlesResult;

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
      Azure::Nullable<std::string> ContinuationToken;
      std::vector<DirectoryItem> DirectoryItems;
      std::vector<FileItem> FileItems;
      std::string RequestId;
    };

    struct ListShareDirectoryHandlesSinglePageResult
    {
      std::vector<HandleItem> Handles;
      Azure::Nullable<std::string> ContinuationToken;
      std::string RequestId;
    };

    // FileClient models:
    struct CreateShareFileResult
    {
      bool Created = true;
      Azure::ETag ETag;
      DateTime LastModified;
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
      DateTime LastModified;
      Storage::Metadata Metadata;
      Azure::ETag ETag;
      Azure::Nullable<DateTime> CopyCompletedOn;
      Azure::Nullable<std::string> CopyStatusDescription;
      Azure::Nullable<std::string> CopyId;
      Azure::Nullable<std::string> CopyProgress;
      Azure::Nullable<std::string> CopySource;
      Azure::Nullable<CopyStatusType> CopyStatus;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      Azure::Nullable<LeaseDurationType> LeaseDuration;
      Azure::Nullable<LeaseStateType> LeaseState;
      Azure::Nullable<LeaseStatusType> LeaseStatus;
    };

    struct DownloadShareFileResult
    {
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t FileSize = 0;
      Azure::Nullable<Storage::ContentHash> TransactionalContentHash;
      FileHttpHeaders HttpHeaders;
      DownloadShareFileDetails Details;
      std::string RequestId;
    };

    using StartCopyShareFileResult = _detail::FileStartCopyResult;
    using AbortCopyShareFileResult = _detail::FileAbortCopyResult;
    using GetShareFilePropertiesResult = _detail::FileGetPropertiesResult;
    using SetShareFilePropertiesResult = _detail::FileSetHttpHeadersResult;
    using ResizeFileResult = _detail::FileSetHttpHeadersResult;
    using SetShareFileMetadataResult = _detail::FileSetMetadataResult;
    using UploadShareFileRangeResult = _detail::FileUploadRangeResult;
    struct ClearShareFileRangeResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      std::string RequestId;
      bool IsServerEncrypted = bool();
    };
    using UploadFileRangeFromUriResult = _detail::FileUploadRangeFromUrlResult;
    using GetShareFileRangeListResult = _detail::FileGetRangeListResult;
    using ListShareFileHandlesSinglePageResult = ListShareDirectoryHandlesSinglePageResult;
    using ForceCloseAllShareFileHandlesSinglePageResult = _detail::FileForceCloseHandlesResult;

    struct DownloadShareFileToResult
    {
      int64_t FileSize = 0;
      Azure::Core::Http::HttpRange ContentRange;
      FileHttpHeaders HttpHeaders;
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

  } // namespace Models

  class StartCopyShareFileOperation
      : public Azure::Core::Operation<Models::GetShareFilePropertiesResult> {
  public:
    std::string RequestId;
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    std::string CopyId;
    Models::CopyStatusType CopyStatus;
    Azure::Nullable<std::string> VersionId;

  public:
    Models::GetShareFilePropertiesResult Value() const override { return m_pollResult; }

    /**
     * @brief Get the raw HTTP response.
     * @return A pointer to #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse* GetRawResponse() const override { return m_rawResponse.get(); }

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

    Azure::Response<Models::GetShareFilePropertiesResult> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> m_rawResponse;
    std::shared_ptr<ShareFileClient> m_fileClient;
    Models::GetShareFilePropertiesResult m_pollResult;

    friend class ShareFileClient;
  };
}}}} // namespace Azure::Storage::Files::Shares

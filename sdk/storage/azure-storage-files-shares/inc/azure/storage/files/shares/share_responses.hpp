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

    using ListSharesSinglePageResult = Details::ServiceListSharesSinglePageResult;
    using SetServicePropertiesResult = Details::ServiceSetPropertiesResult;
    using GetServicePropertiesResult = FileServiceProperties;

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

    struct CreateShareDirectoryResult
    {
      Core::ETag ETag;
      Core::DateTime LastModified;
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

    using GetShareDirectoryPropertiesResult = Details::DirectoryGetPropertiesResult;
    using SetShareDirectoryPropertiesResult = Details::DirectorySetPropertiesResult;
    using SetShareDirectoryMetadataResult = Details::DirectorySetMetadataResult;
    using ForceCloseAllShareDirectoryHandlesSinglePageResult
        = Details::DirectoryForceCloseHandlesResult;

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
      Azure::Core::Nullable<std::string> ContinuationToken;
      std::vector<DirectoryItem> DirectoryItems;
      std::vector<FileItem> FileItems;
      std::string RequestId;
    };

    struct ListShareDirectoryHandlesSinglePageResult
    {
      std::vector<HandleItem> Handles;
      Azure::Core::Nullable<std::string> ContinuationToken;
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
      std::unique_ptr<Azure::IO::BodyStream> BodyStream;
      Azure::Core::Http::Range ContentRange;
      int64_t FileSize = 0;
      Azure::Core::Nullable<Storage::ContentHash> TransactionalContentHash;
      FileHttpHeaders HttpHeaders;
      DownloadShareFileDetails Details;
      std::string RequestId;
    };

    using StartCopyShareFileResult = Details::FileStartCopyResult;
    using AbortCopyShareFileResult = Details::FileAbortCopyResult;
    using GetShareFilePropertiesResult = Details::FileGetPropertiesResult;
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
    using ForceCloseAllShareFileHandlesSinglePageResult = Details::FileForceCloseHandlesResult;

    struct DownloadShareFileToResult
    {
      int64_t FileSize = 0;
      Azure::Core::Http::Range ContentRange;
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
    Azure::Core::ETag ETag;
    Azure::Core::DateTime LastModified;
    std::string CopyId;
    Models::CopyStatusType CopyStatus;
    Azure::Core::Nullable<std::string> VersionId;

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

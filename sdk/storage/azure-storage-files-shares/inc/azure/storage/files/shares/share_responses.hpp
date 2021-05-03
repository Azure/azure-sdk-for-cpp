// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/azure_assert.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/paged_response.hpp>

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_options.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareServiceClient;
  class ShareFileClient;
  class ShareDirectoryClient;

  namespace Models {

    // ServiceClient models:

    using SetServicePropertiesResult = _detail::ServiceSetPropertiesResult;

    // ShareClient models:
    struct CreateShareResult
    {
      bool Created = true;
      Azure::ETag ETag;
      DateTime LastModified;
    };

    struct DeleteShareResult
    {
      bool Deleted = true;
    };
    using CreateShareSnapshotResult = _detail::ShareCreateSnapshotResult;
    using ShareProperties = _detail::ShareGetPropertiesResult;
    using SetSharePropertiesResult = _detail::ShareSetPropertiesResult;
    using SetShareMetadataResult = _detail::ShareSetMetadataResult;
    using ShareAccessPolicy = _detail::ShareGetAccessPolicyResult;
    using SetShareAccessPolicyResult = _detail::ShareSetAccessPolicyResult;
    using ShareStatistics = _detail::ShareGetStatisticsResult;
    using CreateSharePermissionResult = _detail::ShareCreatePermissionResult;
    using AcquireLeaseResult = _detail::ShareAcquireLeaseResult;
    using RenewLeaseResult = _detail::ShareRenewLeaseResult;
    using ReleaseLeaseResult = _detail::ShareReleaseLeaseResult;
    using BreakLeaseResult = _detail::ShareBreakLeaseResult;
    using ChangeLeaseResult = _detail::ShareChangeLeaseResult;

    // DirectoryClient models:

    struct CreateDirectoryResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      std::string ParentFileId;
      bool Created = false;
    };

    struct DeleteDirectoryResult
    {
      bool Deleted = true;
    };

    using DirectoryProperties = _detail::DirectoryGetPropertiesResult;
    using SetDirectoryPropertiesResult = _detail::DirectorySetPropertiesResult;
    using SetDirectoryMetadataResult = _detail::DirectorySetMetadataResult;

    struct ForceCloseDirectoryHandleResult
    {
    };

    // FileClient models:
    struct CreateFileResult
    {
      bool Created = true;
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
    };

    struct DeleteFileResult
    {
      bool Deleted = true;
    };

    struct DownloadFileDetails
    {
      DateTime LastModified;
      Storage::Metadata Metadata;
      Azure::ETag ETag;
      Nullable<DateTime> CopyCompletedOn;
      Nullable<std::string> CopyStatusDescription;
      Nullable<std::string> CopyId;
      Nullable<std::string> CopyProgress;
      Nullable<std::string> CopySource;
      Nullable<Models::CopyStatus> CopyStatus;
      bool IsServerEncrypted = bool();
      FileSmbProperties SmbProperties;
      Nullable<Models::LeaseDuration> LeaseDuration;
      Nullable<Models::LeaseState> LeaseState;
      Nullable<Models::LeaseStatus> LeaseStatus;
    };

    struct DownloadFileResult
    {
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t FileSize = 0;
      Nullable<Storage::ContentHash> TransactionalContentHash;
      FileHttpHeaders HttpHeaders;
      DownloadFileDetails Details;
    };

    using AbortFileCopyResult = _detail::FileAbortCopyResult;
    using FileProperties = _detail::FileGetPropertiesResult;
    using SetFilePropertiesResult = _detail::FileSetHttpHeadersResult;
    using ResizeFileResult = _detail::FileSetHttpHeadersResult;
    using SetFileMetadataResult = _detail::FileSetMetadataResult;
    using UploadFileRangeResult = _detail::FileUploadRangeResult;
    struct ClearFileRangeResult
    {
      Azure::ETag ETag;
      DateTime LastModified;
      bool IsServerEncrypted = bool();
    };
    using UploadFileRangeFromUriResult = _detail::FileUploadRangeFromUrlResult;
    using GetFileRangeListResult = _detail::FileGetRangeListResult;

    struct DownloadFileToResult
    {
      int64_t FileSize = 0;
      Azure::Core::Http::HttpRange ContentRange;
      FileHttpHeaders HttpHeaders;
      DownloadFileDetails Details;
    };

    struct ForceCloseFileHandleResult
    {
    };

    struct UploadFileFromResult
    {
      bool IsServerEncrypted = false;
    };

  } // namespace Models

  class StartFileCopyOperation : public Azure::Core::Operation<Models::FileProperties> {
  public:
    Models::FileProperties Value() const override { return m_pollResult; }

    StartFileCopyOperation() = default;

    StartFileCopyOperation(StartFileCopyOperation&&) = default;

    StartFileCopyOperation& operator=(StartFileCopyOperation&&) = default;

    ~StartFileCopyOperation() override {}

  private:
    std::string GetResumeToken() const override { AZURE_NOT_IMPLEMENTED; }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override;

    Azure::Response<Models::FileProperties> PollUntilDoneInternal(
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
    Models::FileProperties m_pollResult;

    friend class ShareFileClient;
  };

  class ListSharesPagedResponse : public Azure::Core::PagedResponse<ListSharesPagedResponse> {
  public:
    std::string ServiceEndpoint;
    std::string Prefix;
    std::vector<Models::ShareItem> Shares;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareServiceClient> m_shareServiceClient;
    ListSharesOptions m_operationOptions;

    friend class ShareServiceClient;
    friend class PagedResponse<ListSharesPagedResponse>;
  };

  class ListFilesAndDirectoriesPagedResponse
      : public Azure::Core::PagedResponse<ListFilesAndDirectoriesPagedResponse> {
  public:
    std::string ServiceEndpoint;
    std::string ShareName;
    std::string ShareSnapshot;
    std::string DirectoryPath;
    std::string Prefix;
    std::vector<Models::DirectoryItem> Directories;
    std::vector<Models::FileItem> Files;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ListFilesAndDirectoriesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class PagedResponse<ListFilesAndDirectoriesPagedResponse>;
  };

  class ListFileHandlesPagedResponse
      : public Azure::Core::PagedResponse<ListFileHandlesPagedResponse> {
  public:
    std::vector<Models::HandleItem> FileHandles;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareFileClient> m_shareFileClient;
    ListFileHandlesOptions m_operationOptions;

    friend class ShareFileClient;
    friend class PagedResponse<ListFileHandlesPagedResponse>;
  };

  class ForceCloseAllFileHandlesPagedResponse
      : public Azure::Core::PagedResponse<ForceCloseAllFileHandlesPagedResponse> {
  public:
    int32_t NumberOfHandlesClosed = 0;
    int32_t NumberOfHandlesFailedToClose = 0;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareFileClient> m_shareFileClient;
    ForceCloseAllFileHandlesOptions m_operationOptions;

    friend class ShareFileClient;
    friend class PagedResponse<ForceCloseAllFileHandlesPagedResponse>;
  };

  class ListDirectoryHandlesPagedResponse
      : public Azure::Core::PagedResponse<ListDirectoryHandlesPagedResponse> {
  public:
    std::vector<Models::HandleItem> DirectoryHandles;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ListDirectoryHandlesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class PagedResponse<ListDirectoryHandlesPagedResponse>;
  };

  class ForceCloseAllDirectoryHandlesPagedResponse
      : public Azure::Core::PagedResponse<ForceCloseAllDirectoryHandlesPagedResponse> {
  public:
    int32_t NumberOfHandlesClosed = 0;
    int32_t NumberOfHandlesFailedToClose = 0;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ForceCloseAllDirectoryHandlesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class PagedResponse<ForceCloseAllDirectoryHandlesPagedResponse>;
  };

}}}} // namespace Azure::Storage::Files::Shares

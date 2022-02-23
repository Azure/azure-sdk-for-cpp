// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/operation.hpp>
#include <azure/core/paged_response.hpp>

#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_options.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareServiceClient;
  class ShareFileClient;
  class ShareDirectoryClient;

  namespace Models {

    using LeaseDuration [[deprecated]] = LeaseDurationType;

    /**
     * @brief The information returned when forcing the directory handles to close.
     */
    struct ForceCloseDirectoryHandleResult final
    {
    };

    /**
     * @brief The information returned when clearing a range in the file.
     */
    struct ClearFileRangeResult final
    {
      /**
       * An HTTP entity tag associated with the file.
       */
      Azure::ETag ETag;

      /**
       * The data and time the file was last modified.
       */
      DateTime LastModified;

      /**
       * A boolean indicates if the service is encrypted.
       */
      bool IsServerEncrypted = bool();
    };

    /**
     * @brief The information returned when downloading a file to a destination.
     */
    struct DownloadFileToResult final
    {
      /**
       * The size of the file.
       */
      int64_t FileSize = 0;

      /**
       * The range of the downloaded content.
       */
      Azure::Core::Http::HttpRange ContentRange;

      /**
       * The common HTTP headers of the file.
       */
      FileHttpHeaders HttpHeaders;

      /**
       * The detailed information of the downloaded file.
       */
      DownloadFileDetails Details;
    };

    /**
     * @brief The information returned when forcing a file handle to close.
     */
    struct ForceCloseFileHandleResult final
    {
    };

    /**
     * @brief The information returned when uploading a file from a source.
     */
    struct UploadFileFromResult final
    {
      /**
       * A boolean indicates if the service is encrypted.
       */
      bool IsServerEncrypted = false;
    };

    /**
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::AcquireLease.
     */
    struct AcquireLeaseResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally, in
       * quotes.
       */
      Azure::ETag ETag;
      /**
       * Returns the date and time the share was last modified. Any operation that modifies the
       * share or its properties updates the last modified time. Operations on files do not affect
       * the last modified time of the share.
       */
      DateTime LastModified;
      /**
       * Uniquely identifies a share's or file's lease.
       */
      std::string LeaseId;
    };
    /**
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::ReleaseLease.
     */
    struct ReleaseLeaseResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally, in
       * quotes.
       */
      Azure::ETag ETag;
      /**
       * Returns the date and time the share was last modified. Any operation that modifies the
       * share or its properties updates the last modified time. Operations on files do not affect
       * the last modified time of the share.
       */
      DateTime LastModified;
    };
    /**
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::ChangeLease.
     */
    struct ChangeLeaseResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally, in
       * quotes.
       */
      Azure::ETag ETag;
      /**
       * Returns the date and time the share was last modified. Any operation that modifies the
       * share or its properties updates the last modified time. Operations on files do not affect
       * the last modified time of the share.
       */
      DateTime LastModified;
      /**
       * Uniquely identifies a share's or file's lease.
       */
      std::string LeaseId;
    };
    /**
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::RenewLease.
     */
    struct RenewLeaseResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally, in
       * quotes.
       */
      Azure::ETag ETag;
      /**
       * Returns the date and time the share was last modified. Any operation that modifies the
       * share or its properties updates the last modified time. Operations on files do not affect
       * the last modified time of the share.
       */
      DateTime LastModified;
      /**
       * Uniquely identifies a share's or file's lease.
       */
      std::string LeaseId;
    };
    /**
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::BreakLease.
     */
    struct BreakLeaseResult final
    {
      /**
       * The ETag contains a value that you can use to perform operations conditionally, in
       * quotes.
       */
      Azure::ETag ETag;
      /**
       * Returns the date and time the share was last modified. Any operation that modifies the
       * share or its properties updates the last modified time. Operations on files do not affect
       * the last modified time of the share.
       */
      DateTime LastModified;
      /**
       * Approximate time remaining in the lease period, in seconds.
       */
      int32_t LeaseTime = int32_t();
      /**
       * Uniquely identifies a share's or file's lease.
       */
      std::string LeaseId;
    };
  } // namespace Models

  /**
   * @brief A long-running operation to copy a file.
   */
  class StartFileCopyOperation final : public Azure::Core::Operation<Models::FileProperties> {
  public:
    /**
     * @brief Get the #Azure::Storage::Files::Shares::Models::FileProperties object which includes
     * the latest copy information.
     *
     * @return An #Azure::Storage::Files::Shares::Models::FileProperties object.
     */
    Models::FileProperties Value() const override { return m_pollResult; }

    StartFileCopyOperation() = default;

    StartFileCopyOperation(StartFileCopyOperation&&) = default;

    StartFileCopyOperation& operator=(StartFileCopyOperation&&) = default;

    ~StartFileCopyOperation() override {}

  private:
    std::string GetResumeToken() const override { AZURE_NOT_IMPLEMENTED(); }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        const Azure::Core::Context& context) override;

    Azure::Response<Models::FileProperties> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    const Azure::Core::Http::RawResponse& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::shared_ptr<ShareFileClient> m_fileClient;
    Models::FileProperties m_pollResult;

    friend class ShareFileClient;
  };

  /**
   * @brief Response type for #Azure::Storage::Files::Shares::ShareServiceClient::ListShares.
   */
  class ListSharesPagedResponse final : public Azure::Core::PagedResponse<ListSharesPagedResponse> {
  public:
    /**
     * Service endpoint.
     */
    std::string ServiceEndpoint;
    /**
     * Share name prefix that's used to filter the result.
     */
    std::string Prefix;
    /**
     * File share items.
     */
    std::vector<Models::ShareItem> Shares;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareServiceClient> m_shareServiceClient;
    ListSharesOptions m_operationOptions;

    friend class ShareServiceClient;
    friend class Azure::Core::PagedResponse<ListSharesPagedResponse>;
  };

  /**
   * @brief Response type for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ListFilesAndDirectories.
   */
  class ListFilesAndDirectoriesPagedResponse final
      : public Azure::Core::PagedResponse<ListFilesAndDirectoriesPagedResponse> {
  public:
    /**
     * Service endpoint.
     */
    std::string ServiceEndpoint;
    /**
     * Name of the file share.
     */
    std::string ShareName;
    /**
     * The share snapshot for the list operation.
     */
    std::string ShareSnapshot;
    /**
     * Directory path for the list operation.
     */
    std::string DirectoryPath;
    /**
     * Name prefix that's used to filter the result.
     */
    std::string Prefix;
    /**
     * Directory items.
     */
    std::vector<Models::DirectoryItem> Directories;
    /**
     * File items.
     */
    std::vector<Models::FileItem> Files;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ListFilesAndDirectoriesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class Azure::Core::PagedResponse<ListFilesAndDirectoriesPagedResponse>;
  };

  /**
   * @brief Response type for #Azure::Storage::Files::Shares::ShareFileClient::ListHandles.
   */
  class ListFileHandlesPagedResponse final
      : public Azure::Core::PagedResponse<ListFileHandlesPagedResponse> {
  public:
    /**
     * File handles.
     */
    std::vector<Models::HandleItem> FileHandles;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareFileClient> m_shareFileClient;
    ListFileHandlesOptions m_operationOptions;

    friend class ShareFileClient;
    friend class Azure::Core::PagedResponse<ListFileHandlesPagedResponse>;
  };

  /**
   * @brief Response type for #Azure::Storage::Files::Shares::ShareFileClient::ForceCloseAllHandles.
   */
  class ForceCloseAllFileHandlesPagedResponse final
      : public Azure::Core::PagedResponse<ForceCloseAllFileHandlesPagedResponse> {
  public:
    /**
     * Number of file handles that were closed.
     */
    int32_t NumberOfHandlesClosed = 0;
    /**
     * Number of file handles that failed to close.
     */
    int32_t NumberOfHandlesFailedToClose = 0;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareFileClient> m_shareFileClient;
    ForceCloseAllFileHandlesOptions m_operationOptions;

    friend class ShareFileClient;
    friend class Azure::Core::PagedResponse<ForceCloseAllFileHandlesPagedResponse>;
  };

  /**
   * @brief Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::ListHandles.
   */
  class ListDirectoryHandlesPagedResponse final
      : public Azure::Core::PagedResponse<ListDirectoryHandlesPagedResponse> {
  public:
    /**
     * File handles.
     */
    std::vector<Models::HandleItem> DirectoryHandles;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ListDirectoryHandlesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class Azure::Core::PagedResponse<ListDirectoryHandlesPagedResponse>;
  };

  /**
   * @brief Response type for
   * #Azure::Storage::Files::Shares::ShareDirectoryClient::ForceCloseAllHandles.
   */
  class ForceCloseAllDirectoryHandlesPagedResponse final
      : public Azure::Core::PagedResponse<ForceCloseAllDirectoryHandlesPagedResponse> {
  public:
    /**
     * Number of file handles that were closed.
     */
    int32_t NumberOfHandlesClosed = 0;
    /**
     * Number of file handles that failed to close.
     */
    int32_t NumberOfHandlesFailedToClose = 0;

  private:
    void OnNextPage(const Azure::Core::Context& context);

    std::shared_ptr<ShareDirectoryClient> m_shareDirectoryClient;
    ForceCloseAllDirectoryHandlesOptions m_operationOptions;

    friend class ShareDirectoryClient;
    friend class Azure::Core::PagedResponse<ForceCloseAllDirectoryHandlesPagedResponse>;
  };

}}}} // namespace Azure::Storage::Files::Shares

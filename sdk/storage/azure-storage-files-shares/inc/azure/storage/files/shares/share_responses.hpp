// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

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

    /**
     * @brief The information returned when forcing the directory handles to close.
     */
    struct ForceCloseDirectoryHandleResult
    {
    };

    /**
     * @brief The detailed information returned when downloading a file.
     */
    struct DownloadFileDetails
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
       * The metadata of the file.
       */
      Storage::Metadata Metadata;

      /**
       * The copy completed time of the file, if the file is created from a copy operation.
       */
      Nullable<DateTime> CopyCompletedOn;

      /**
       * The copy status's description of the file, if the file is created from a copy operation.
       */
      Nullable<std::string> CopyStatusDescription;

      /**
       * The copy ID of the file, if the file is created from a copy operation.
       */
      Nullable<std::string> CopyId;

      /**
       * The copy progress of the file, if the file is created from a copy operation.
       */
      Nullable<std::string> CopyProgress;

      /**
       * The copy source of the file, if the file is created from a copy operation.
       */
      Nullable<std::string> CopySource;

      /**
       * The copy status of the file, if the file is created from a copy operation.
       */
      Nullable<Models::CopyStatus> CopyStatus;

      /**
       * A boolean indicates if the service is encrypted.
       */
      bool IsServerEncrypted = bool();

      /**
       * The SMB related properties of the file or directory.
       */
      FileSmbProperties SmbProperties;

      /**
       * When a file is leased, specifies whether the lease is of infinite or fixed duration.
       */
      Nullable<Models::LeaseDuration> LeaseDuration;

      /**
       * Lease state of the file.
       */
      Nullable<Models::LeaseState> LeaseState;

      /**
       * The current lease status of the file.
       */
      Nullable<Models::LeaseStatus> LeaseStatus;
    };

    /**
     * @brief The content and information returned when downloading a file.
     */
    struct DownloadFileResult
    {
      /**
       * The body of the downloaded result.
       */
      std::unique_ptr<Azure::Core::IO::BodyStream> BodyStream;

      /**
       * The range of the downloaded content.
       */
      Azure::Core::Http::HttpRange ContentRange;

      /**
       * The size of the file.
       */
      int64_t FileSize = 0;

      /**
       * The transactional hash of the downloaded content.
       */
      Nullable<Storage::ContentHash> TransactionalContentHash;

      /**
       * The common Http headers of the file.
       */
      FileHttpHeaders HttpHeaders;

      /**
       * The detailed information of the downloaded file.
       */
      DownloadFileDetails Details;
    };

    /**
     * @brief The information returned when clearing a range in the file.
     */
    struct ClearFileRangeResult
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
    struct DownloadFileToResult
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
       * The common Http headers of the file.
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
    struct ForceCloseFileHandleResult
    {
    };

    /**
     * @brief The information returned when uploading a file from a source.
     */
    struct UploadFileFromResult
    {
      /**
       * A boolean indicates if the service is encrypted.
       */
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
    std::string GetResumeToken() const override
    {
      // Not supported
      std::abort();
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        const Azure::Core::Context& context) override;

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

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/files/shares/share_constants.hpp"
#include "azure/storage/files/shares/share_options.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/paged_response.hpp>

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
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::Acquire.
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
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::Release.
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
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::Change.
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
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::Renew.
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
     * @brief Response type for #Azure::Storage::Files::Shares::ShareLeaseClient::Break.
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
    };

    /**
     * @brief Access rights of the handle.
     */
    class ShareFileHandleAccessRights final {
    public:
      ShareFileHandleAccessRights() = default;
      /**
       * @brief Create from a string.
       * @param value A string describing the access rights.
       */
      explicit ShareFileHandleAccessRights(const std::string& value);
      /** @brief Compare two values for equality */
      bool operator==(const ShareFileHandleAccessRights& other) const
      {
        return m_value == other.m_value;
      }
      /** @brief Compare two values for inequality */
      bool operator!=(const ShareFileHandleAccessRights& other) const { return !(*this == other); }
      /** @brief Return the values of the FileAttributes as an array. */
      const std::set<std::string>& GetValues() const { return m_value; }
      /** @brief Bitwise OR of two values*/
      ShareFileHandleAccessRights operator|(const ShareFileHandleAccessRights& other) const;
      /** @brief Bitwise AND of two values*/
      ShareFileHandleAccessRights operator&(const ShareFileHandleAccessRights& other) const;
      /** @brief Bitwise XOR of two values*/
      ShareFileHandleAccessRights operator^(const ShareFileHandleAccessRights& other) const;
      /** @brief Bitwise OR and assignment of two values*/
      ShareFileHandleAccessRights& operator|=(const ShareFileHandleAccessRights& other)
      {
        *this = *this | other;
        return *this;
      }
      /** @brief Bitwise AND and assignment of two values*/
      ShareFileHandleAccessRights& operator&=(const ShareFileHandleAccessRights& other)
      {
        *this = *this & other;
        return *this;
      }
      /** @brief Bitwise XOR and assignment of two values*/
      ShareFileHandleAccessRights& operator^=(const ShareFileHandleAccessRights& other)
      {
        *this = *this ^ other;
        return *this;
      }
      /** @brief Read access rights */
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static ShareFileHandleAccessRights Read;
      /** @brief Write access rights */
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static ShareFileHandleAccessRights Write;
      /** @brief Delete access rights */
      AZ_STORAGE_FILES_SHARES_DLLEXPORT const static ShareFileHandleAccessRights Delete;

    private:
      std::set<std::string> m_value;
    };

    /**
     * @brief A listed directory item.
     */
    struct DirectoryItem final
    {
      /** @brief The name of the item */
      std::string Name;
      /**
       * File properties.
       */
      DirectoryItemDetails Details;
    };

    /**
     * @brief A listed file item.
     */
    struct FileItem final
    {
      /** @brief The name of the item */
      std::string Name;
      /**
       * File properties.
       */
      FileItemDetails Details;
    };

    /**
     * @brief A listed Azure Storage handle item.
     */
    struct HandleItem final
    {
      /**
       * XSMB service handle ID.
       */
      std::string HandleId;
      /**
       * File or directory name including full path starting from share root.
       */
      std::string Path;
      /**
       * FileId uniquely identifies the file or directory.
       */
      std::string FileId;
      /**
       * ParentId uniquely identifies the parent directory of the object.
       */
      std::string ParentId;
      /**
       * SMB session ID in context of which the file handle was opened.
       */
      std::string SessionId;
      /**
       * Client IP that opened the handle.
       */
      std::string ClientIp;
      /**
       * Name of the client machine where the share is being mounted.
       */
      std::string ClientName;
      /**
       * Time when the session that previously opened the handle has last been reconnected. (UTC).
       */
      DateTime OpenedOn;
      /**
       * Time handle was last connected to (UTC).
       */
      DateTime LastReconnectedOn;
      /**
       * Access rights of the handle.
       */
      Azure::Nullable<ShareFileHandleAccessRights> AccessRights;
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

    /** @brief Construct a new StartFileCopyOperation object moving from another
     * StartFileCopyOperation object. */
    StartFileCopyOperation(StartFileCopyOperation&&) = default;

    /** @brief Move a StartFileCopyOperation to another. */
    StartFileCopyOperation& operator=(StartFileCopyOperation&&) = default;

    ~StartFileCopyOperation() override {}

  private:
    std::string GetResumeToken() const override { AZURE_NOT_IMPLEMENTED(); }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        const Azure::Core::Context& context) override;

    Azure::Response<Models::FileProperties> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

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
    /**
     * FileId of the directory.
     */
    std::string DirectoryId;

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

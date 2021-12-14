// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <azure/core/operation.hpp>
#include <azure/core/paged_response.hpp>

#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage {

  namespace Files { namespace DataLake {
    class ListFileSystemsPagedResponse;
  }} // namespace Files::DataLake

  namespace Blobs {

    class BlobServiceClient;
    class BlobContainerClient;
    class BlobClient;
    class PageBlobClient;

    namespace Models {

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobClient::DownloadTo.
       */
      struct DownloadBlobToResult final
      {
        /**
         * The blob's type.
         */
        Models::BlobType BlobType;

        /**
         * Indicates the range of bytes returned.
         */
        Azure::Core::Http::HttpRange ContentRange;

        /**
         * Size of the blob.
         */
        int64_t BlobSize = 0;

        /**
         * The request may return a CRC64 or MD5 hash for the downloaded range of data.
         */
        Azure::Nullable<ContentHash> TransactionalContentHash;

        /**
         * Details information of the downloaded blob.
         */
        DownloadBlobDetails Details;
      };

      using UploadBlockBlobFromResult = UploadBlockBlobResult;

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobLeaseClient::Acquire.
       */
      struct AcquireLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;

        /**
         * The date/time that the blob was last modified. The date format follows RFC 1123.
         */
        Azure::DateTime LastModified;

        /**
         * When you request a lease, the Blob service returns a unique lease ID. While the lease is
         * active, you must include the lease ID with any request to write to the blob, or to renew,
         * change, or release the lease.
         */
        std::string LeaseId;
      };

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobLeaseClient::Break.
       */
      struct BreakLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;

        /**
         * The date/time that the blob was last modified. The date format follows RFC 1123.
         */
        Azure::DateTime LastModified;
      };

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobLeaseClient::Change.
       */
      struct ChangeLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;

        /**
         * The date/time that the blob was last modified. The date format follows RFC 1123.
         */
        Azure::DateTime LastModified;

        /**
         * When you request a lease, the Blob service returns a unique lease ID. While the lease is
         * active, you must include the lease ID with any request to write to the blob, or to renew,
         * change, or release the lease.
         */
        std::string LeaseId;
      };

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobLeaseClient::Release.
       */
      struct ReleaseLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;

        /**
         * The date/time that the blob was last modified. The date format follows RFC 1123.
         */
        Azure::DateTime LastModified;
      };

      /**
       * @brief Response type for #Azure::Storage::Blobs::BlobLeaseClient::Renew.
       */
      struct RenewLeaseResult final
      {
        /**
         * The ETag contains a value that you can use to perform operations conditionally.
         */
        Azure::ETag ETag;

        /**
         * The date/time that the blob was last modified. The date format follows RFC 1123.
         */
        Azure::DateTime LastModified;

        /**
         * When you request a lease, the Blob service returns a unique lease ID. While the lease is
         * active, you must include the lease ID with any request to write to the blob, or to renew,
         * change, or release the lease.
         */
        std::string LeaseId;
      };

    } // namespace Models

    /**
     * @brief A long-running operation to copy a blob.
     */
    class StartBlobCopyOperation final : public Azure::Core::Operation<Models::BlobProperties> {
    public:
      /**
       * @brief Get the #Azure::Storage::Blobs::Models::BlobProperties object, which includes the
       * latest copy information.
       *
       * @return The #Azure::Storage::Blobs::Models::BlobProperties object.
       */
      Models::BlobProperties Value() const override { return m_pollResult; }

      StartBlobCopyOperation() = default;

      StartBlobCopyOperation(StartBlobCopyOperation&&) = default;

      StartBlobCopyOperation& operator=(StartBlobCopyOperation&&) = default;

      ~StartBlobCopyOperation() override {}

    private:
      std::string GetResumeToken() const override { _azure_NOT_IMPLEMENTED(); }

      std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
          const Azure::Core::Context& context) override;

      Azure::Response<Models::BlobProperties> PollUntilDoneInternal(
          std::chrono::milliseconds period,
          Azure::Core::Context& context) override;

      Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
      {
        return *m_rawResponse;
      }

      std::shared_ptr<BlobClient> m_blobClient;
      Models::BlobProperties m_pollResult;

      friend class Blobs::BlobClient;
      friend class Blobs::PageBlobClient;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobServiceClient::ListBlobContainers.
     */
    class ListBlobContainersPagedResponse final
        : public Azure::Core::PagedResponse<ListBlobContainersPagedResponse> {
    public:
      /**
       * Service endpoint.
       */
      std::string ServiceEndpoint;

      /**
       * Container name prefix that's used to filter the result.
       */
      std::string Prefix;

      /**
       * Blob container items.
       */
      std::vector<Models::BlobContainerItem> BlobContainers;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<BlobServiceClient> m_blobServiceClient;
      ListBlobContainersOptions m_operationOptions;

      friend class BlobServiceClient;
      friend class Azure::Core::PagedResponse<ListBlobContainersPagedResponse>;
      friend class Files::DataLake::ListFileSystemsPagedResponse;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobServiceClient::FindBlobsByTags.
     */
    class FindBlobsByTagsPagedResponse final
        : public Azure::Core::PagedResponse<FindBlobsByTagsPagedResponse> {
    public:
      /**
       * Service endpoint.
       */
      std::string ServiceEndpoint;

      /**
       * Blob items filtered by tag.
       */
      std::vector<Models::TaggedBlobItem> TaggedBlobs;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<BlobServiceClient> m_blobServiceClient;
      FindBlobsByTagsOptions m_operationOptions;
      std::string m_tagFilterSqlExpression;

      friend class BlobServiceClient;
      friend class Azure::Core::PagedResponse<FindBlobsByTagsPagedResponse>;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::ListBlobs.
     */
    class ListBlobsPagedResponse final : public Azure::Core::PagedResponse<ListBlobsPagedResponse> {
    public:
      /**
       * Service endpoint.
       */
      std::string ServiceEndpoint;

      /**
       * Name of the container.
       */
      std::string BlobContainerName;

      /**
       * Blob name prefix that's used to filter the result.
       */
      std::string Prefix;

      /**
       * Blob items.
       */
      std::vector<Models::BlobItem> Blobs;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<BlobContainerClient> m_blobContainerClient;
      ListBlobsOptions m_operationOptions;

      friend class BlobContainerClient;
      friend class Azure::Core::PagedResponse<ListBlobsPagedResponse>;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::BlobContainerClient::ByHierarchy.
     */
    class ListBlobsByHierarchyPagedResponse final
        : public Azure::Core::PagedResponse<ListBlobsByHierarchyPagedResponse> {
    public:
      /**
       * Service endpoint.
       */
      std::string ServiceEndpoint;

      /**
       * Name of the container.
       */
      std::string BlobContainerName;

      /**
       * Blob name prefix that's used to filter the result.
       */
      std::string Prefix;

      /**
       * A character or a string used to traverse a virtual hierarchy of blobs as though it were a
       * file system.
       */
      std::string Delimiter;

      /**
       * Blob items.
       */
      std::vector<Models::BlobItem> Blobs;

      /**
       * Blob prefix items.
       */
      std::vector<std::string> BlobPrefixes;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<BlobContainerClient> m_blobContainerClient;
      ListBlobsOptions m_operationOptions;
      std::string m_delimiter;

      friend class BlobContainerClient;
      friend class Azure::Core::PagedResponse<ListBlobsByHierarchyPagedResponse>;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::GetPageRanges.
     */
    class GetPageRangesPagedResponse final
        : public Azure::Core::PagedResponse<GetPageRangesPagedResponse> {
    public:
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;

      /**
       * The date/time that the blob was last modified. The date format follows RFC 1123.
       */
      Azure::DateTime LastModified;

      /**
       * Size of the blob.
       */
      int64_t BlobSize = 0;

      /**
       * Page range items.
       */
      std::vector<Azure::Core::Http::HttpRange> PageRanges;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<PageBlobClient> m_pageBlobClient;
      GetPageRangesOptions m_operationOptions;

      friend class PageBlobClient;
      friend class Azure::Core::PagedResponse<GetPageRangesPagedResponse>;
    };

    /**
     * @brief Response type for #Azure::Storage::Blobs::PageBlobClient::GetPageRangesDiff and
     * #Azure::Storage::Blobs::PageBlobClient::GetManagedDiskPageRangesDiff.
     */
    class GetPageRangesDiffPagedResponse final
        : public Azure::Core::PagedResponse<GetPageRangesDiffPagedResponse> {
    public:
      /**
       * The ETag contains a value that you can use to perform operations conditionally.
       */
      Azure::ETag ETag;

      /**
       * The date/time that the blob was last modified. The date format follows RFC 1123.
       */
      Azure::DateTime LastModified;

      /**
       * Size of the blob.
       */
      int64_t BlobSize = 0;

      /**
       * Page range items.
       */
      std::vector<Azure::Core::Http::HttpRange> PageRanges;

      /**
       * Clear range items.
       */
      std::vector<Azure::Core::Http::HttpRange> ClearRanges;

    private:
      void OnNextPage(const Azure::Core::Context& context);

      std::shared_ptr<PageBlobClient> m_pageBlobClient;
      GetPageRangesOptions m_operationOptions;
      Azure::Nullable<std::string> m_previousSnapshot;
      Azure::Nullable<std::string> m_previousSnapshotUrl;

      friend class PageBlobClient;
      friend class Azure::Core::PagedResponse<GetPageRangesDiffPagedResponse>;
    };

  } // namespace Blobs
}} // namespace Azure::Storage

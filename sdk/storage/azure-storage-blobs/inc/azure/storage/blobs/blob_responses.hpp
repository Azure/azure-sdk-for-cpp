// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <azure/core/operation.hpp>
#include <azure/storage/common/paged_response.hpp>

#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  class BlobContainerClient;
  class BlobClient;
  class PageBlobClient;

  namespace Models {

    struct DownloadBlobToResult
    {
      Models::BlobType BlobType;
      Azure::Core::Http::HttpRange ContentRange;
      int64_t BlobSize = 0;
      Azure::Nullable<ContentHash> TransactionalContentHash; // hash for the downloaded range
      DownloadBlobDetails Details;
    };

    using UploadBlockBlobFromResult = UploadBlockBlobResult;

    struct AcquireLeaseResult
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

    struct BreakLeaseResult
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    };

    struct ChangeLeaseResult
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

    struct ReleaseLeaseResult
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    };

    struct RenewLeaseResult
    {
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

  } // namespace Models

  class StartBlobCopyOperation : public Azure::Core::Operation<Models::BlobProperties> {
  public:
    Models::BlobProperties Value() const override { return m_pollResult; }

    StartBlobCopyOperation() = default;

    StartBlobCopyOperation(StartBlobCopyOperation&&) = default;

    StartBlobCopyOperation& operator=(StartBlobCopyOperation&&) = default;

    ~StartBlobCopyOperation() override {}

  private:
    std::string GetResumeToken() const override
    {
      // Not supported
      std::abort();
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override;

    Azure::Response<Models::BlobProperties> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    /**
     * @brief Get the raw HTTP response.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

    std::shared_ptr<BlobClient> m_blobClient;
    Models::BlobProperties m_pollResult;

    friend class Blobs::BlobClient;
    friend class Blobs::PageBlobClient;
  };

  class ListBlobsPageResult : public _internal::PageResult<ListBlobsPageResult> {
  public:
    std::string ServiceEndpoint;
    std::string BlobContainerName;
    std::string Prefix;
    std::vector<Models::BlobItem> Items;

  private:
    std::shared_ptr<BlobContainerClient> m_blobContainerClient;
    ListBlobsOptions m_operationOptions;
    Azure::Core::Context m_context;

    void OnNextPage(const Azure::Core::Context& context);

    friend class Blobs::BlobContainerClient;
    friend _internal::PageResult<ListBlobsPageResult>;
  };
}}} // namespace Azure::Storage::Blobs

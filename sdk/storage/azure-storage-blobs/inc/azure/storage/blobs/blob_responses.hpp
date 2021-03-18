// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <azure/core/operation.hpp>

#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

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
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

    struct BreakLeaseResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    };

    struct ChangeLeaseResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

    struct ReleaseLeaseResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
    };

    struct RenewLeaseResult
    {
      std::string RequestId;
      Azure::ETag ETag;
      Azure::DateTime LastModified;
      std::string LeaseId;
    };

  } // namespace Models

  class StartCopyBlobOperation : public Azure::Core::Operation<Models::BlobProperties> {
  public:
    std::string RequestId;
    Azure::ETag ETag;
    Azure::DateTime LastModified;
    std::string CopyId;
    Models::CopyStatus CopyStatus;
    Azure::Nullable<std::string> VersionId;

  public:
    Models::BlobProperties Value() const override { return m_pollResult; }

    StartCopyBlobOperation() = default;

    StartCopyBlobOperation(StartCopyBlobOperation&&) = default;

    StartCopyBlobOperation& operator=(StartCopyBlobOperation&&) = default;

    ~StartCopyBlobOperation() override {}

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
}}} // namespace Azure::Storage::Blobs

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
      Azure::Core::Http::Range ContentRange;
      int64_t BlobSize = 0;
      Azure::Core::Nullable<ContentHash> TransactionalContentHash; // hash for the downloaded range
      DownloadBlobDetails Details;
    };

    using UploadBlockBlobFromResult = UploadBlockBlobResult;

    struct AcquireBlobLeaseResult
    {
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      std::string LeaseId;
    };

    struct BreakBlobLeaseResult
    {
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      int32_t LeaseTime = 0;
    };

    struct ChangeBlobLeaseResult
    {
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      std::string LeaseId;
    };

    struct ReleaseBlobLeaseResult
    {
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
    };

    struct RenewBlobLeaseResult
    {
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      std::string LeaseId;
    };

    class StartCopyBlobResult : public Azure::Core::Operation<GetBlobPropertiesResult> {
    public:
      std::string RequestId;
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      std::string CopyId;
      Models::CopyStatus CopyStatus;
      Azure::Core::Nullable<std::string> VersionId;

    public:
      GetBlobPropertiesResult Value() const override { return m_pollResult; }

      ~StartCopyBlobResult() override {}

    private:
      std::string GetResumeToken() const override
      {
        // Not supported
        std::abort();
      }

      std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
          Azure::Core::Context& context) override;

      Azure::Core::Response<GetBlobPropertiesResult> PollUntilDoneInternal(
          Azure::Core::Context& context,
          std::chrono::milliseconds period) override;

      std::shared_ptr<BlobClient> m_blobClient;
      Models::GetBlobPropertiesResult m_pollResult;

      friend class Blobs::BlobClient;
      friend class Blobs::PageBlobClient;
    };

  } // namespace Models
}}} // namespace Azure::Storage::Blobs

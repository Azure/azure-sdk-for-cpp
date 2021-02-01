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
      Azure::Core::ETag ETag;
      Azure::Core::DateTime LastModified;
      Azure::Core::DateTime CreatedOn;
      Azure::Core::Nullable<Azure::Core::DateTime> ExpiresOn;
      Azure::Core::Nullable<Azure::Core::DateTime> LastAccessedOn;
      Azure::Core::Http::Range ContentRange;
      int64_t BlobSize = 0;
      BlobHttpHeaders HttpHeaders;
      Storage::Metadata Metadata;
      Azure::Core::Nullable<int64_t> SequenceNumber; // only for page blob
      Azure::Core::Nullable<int64_t> CommittedBlockCount; // only for append blob
      Azure::Core::Nullable<bool> IsSealed; // only for append blob
      Models::BlobType BlobType;
      Azure::Core::Nullable<ContentHash> TransactionalContentHash; // hash for the downloaded range
      Azure::Core::Nullable<BlobLeaseDurationType> LeaseDuration;
      Azure::Core::Nullable<BlobLeaseState> LeaseState;
      Azure::Core::Nullable<BlobLeaseStatus> LeaseStatus;
      bool IsServerEncrypted = false;
      Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
      Azure::Core::Nullable<std::string> EncryptionScope;
      Azure::Core::Nullable<std::string>
          ObjectReplicationDestinationPolicyId; // only valid for replication destination blob
      std::vector<ObjectReplicationPolicy>
          ObjectReplicationSourceProperties; // only valid for replication source blob
      Azure::Core::Nullable<int32_t> TagCount;
      Azure::Core::Nullable<std::string> CopyId;
      Azure::Core::Nullable<std::string> CopySource;
      Azure::Core::Nullable<Models::CopyStatus> CopyStatus;
      Azure::Core::Nullable<std::string> CopyStatusDescription;
      Azure::Core::Nullable<std::string> CopyProgress;
      Azure::Core::Nullable<Azure::Core::DateTime> CopyCompletedOn;
      Azure::Core::Nullable<std::string> VersionId;
      Azure::Core::Nullable<bool> IsCurrentVersion;
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

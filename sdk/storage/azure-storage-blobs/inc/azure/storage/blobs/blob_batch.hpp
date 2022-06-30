// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstring>
#include <future>
#include <memory>
#include <string>
#include <utility>

#include <azure/core/nullable.hpp>
#include <azure/core/url.hpp>

#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/blobs/blob_service_client.hpp"
#include "azure/storage/blobs/deferred_response.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace _detail {
    extern const Core::Context::Key s_batchKey;

    class StringBodyStream final : public Core::IO::BodyStream {
    public:
      explicit StringBodyStream(std::string content) : m_content(std::move(content)) {}
      StringBodyStream(const StringBodyStream&) = delete;
      StringBodyStream& operator=(const StringBodyStream&) = delete;
      StringBodyStream(StringBodyStream&& other) = default;
      StringBodyStream& operator=(StringBodyStream&& other) = default;
      ~StringBodyStream() override {}
      int64_t Length() const override { return m_content.length(); }
      void Rewind() override { m_offset = 0; }

    private:
      size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override;

    private:
      std::string m_content;
      size_t m_offset = 0;
    };

    enum class BatchSubrequestType
    {
      DeleteBlob,
      SetBlobAccessTier,
    };

    struct BatchSubrequest
    {
      explicit BatchSubrequest(BatchSubrequestType type) : Type(type) {}
      virtual ~BatchSubrequest() = 0;

      BatchSubrequestType Type;
    };

    struct BlobBatchAccessHelper;

    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> ConstructBatchRequestPolicy(
        const std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>&
            servicePerRetryPolicies,
        const std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>&
            servicePerOperationPolicies,
        const BlobClientOptions& options);

    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> ConstructBatchSubrequestPolicy(
        std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>&& tokenAuthPolicy,
        std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>&& sharedKeyAuthPolicy,
        const BlobClientOptions& options);
  } // namespace _detail

  class BlobBatch final {
  public:
    DeferredResponse<Models::DeleteBlobResult> DeleteBlob(
        const std::string& blobContainerName,
        const std::string& blobName,
        const DeleteBlobOptions& options = DeleteBlobOptions());

    DeferredResponse<Models::DeleteBlobResult> DeleteBlob(
        const std::string& blobUrl,
        const DeleteBlobOptions& options = DeleteBlobOptions());

    DeferredResponse<Models::SetBlobAccessTierResult> SetBlobAccessTier(
        const std::string& blobContainerName,
        const std::string& blobName,
        Models::AccessTier tier,
        const SetBlobAccessTierOptions& options = SetBlobAccessTierOptions());

    DeferredResponse<Models::SetBlobAccessTierResult> SetBlobAccessTier(
        const std::string& blobUrl,
        Models::AccessTier tier,
        const SetBlobAccessTierOptions& options = SetBlobAccessTierOptions());

  private:
    explicit BlobBatch(BlobServiceClient blobServiceClient);
    explicit BlobBatch(BlobContainerClient blobContainerClient);

    BlobClient GetBlobClientForSubrequest(Core::Url url) const;

  private:
    Core::Url m_url;
    Nullable<BlobServiceClient> m_blobServiceClient;
    Nullable<BlobContainerClient> m_blobContainerClient;

    std::vector<std::shared_ptr<_detail::BatchSubrequest>> m_subrequests;

    friend class BlobServiceClient;
    friend class BlobContainerClient;
    friend struct _detail::BlobBatchAccessHelper;
  };

}}} // namespace Azure::Storage::Blobs

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/nullable.hpp>
#include <azure/core/url.hpp>

#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/blobs/blob_service_client.hpp"
#include "azure/storage/blobs/deferred_response.hpp"
#include "azure/storage/blobs/rtti.hpp"

namespace Azure { namespace Storage { namespace Blobs {

#if defined(AZ_STORAGE_BLOBS_RTTI)

  class BlobBatch final : private _detail::DeferredResponseFactory {
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
    BlobBatch(Core::Url url) : m_url(std::move(url)) {}
    Core::Url m_url;

    friend class BlobBatchClient;
  };

  class BlobBatchClient final {
  public:
    explicit BlobBatchClient(BlobServiceClient blobServiceClient);
    explicit BlobBatchClient(BlobContainerClient blobContainerClient);

    BlobBatch CreateBatch();

    Response<Models::SubmitBlobBatchResult> SubmitBatch(
        const BlobBatch& batch,
        const SubmitBlobBatchOptions& options = SubmitBlobBatchOptions(),
        const Core::Context& context = Core::Context()) const;

  private:
    template <class T> void Init(const T& blobServiceOrContainerClient);
    void ConstructSubrequests(Core::Http::Request& request, const Core::Context& context);
    void ParseSubresponses(
        std::unique_ptr<Core::Http::RawResponse>& rawResponse,
        const Core::Context& context);

  private:
    Nullable<BlobServiceClient> m_blobServiceClient;
    Nullable<BlobContainerClient> m_blobContainerClient;

    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_subrequestPipeline;
  };

#endif
}}} // namespace Azure::Storage::Blobs

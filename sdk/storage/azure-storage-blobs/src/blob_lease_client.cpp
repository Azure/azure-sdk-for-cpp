// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_lease_client.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/uuid.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  const std::chrono::seconds BlobLeaseClient::InfiniteLeaseDuration{-1};

  std::string BlobLeaseClient::CreateUniqueLeaseId()
  {
    return Azure::Core::Uuid::CreateUuid().ToString();
  }

  Azure::Response<Models::AcquireLeaseResult> BlobLeaseClient::Acquire(
      std::chrono::seconds duration,
      const AcquireLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    if (m_blobClient.HasValue())
    {
      _detail::BlobRestClient::Blob::AcquireBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = GetLeaseId();
      protocolLayerOptions.LeaseDuration = duration;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = _detail::BlobRestClient::Blob::AcquireLease(
          *(m_blobClient.Value().m_pipeline),
          m_blobClient.Value().m_blobUrl,
          protocolLayerOptions,
          context);

      Models::AcquireLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::AcquireLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_blobContainerClient.HasValue())
    {
      _detail::BlobRestClient::BlobContainer::AcquireBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = GetLeaseId();
      protocolLayerOptions.LeaseDuration = duration;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfMatch.HasValue(),
          "Blob container lease doesn't support If-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfNoneMatch.HasValue(),
          "Blob container lease doesn't support If-None-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.TagConditions.HasValue(),
          "Blob container lease doesn't support tag condition.");

      auto response = _detail::BlobRestClient::BlobContainer::AcquireLease(
          *(m_blobContainerClient.Value().m_pipeline),
          m_blobContainerClient.Value().m_blobContainerUrl,
          protocolLayerOptions,
          context);

      Models::AcquireLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::AcquireLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  Azure::Response<Models::RenewLeaseResult> BlobLeaseClient::Renew(
      const RenewLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    if (m_blobClient.HasValue())
    {
      _detail::BlobRestClient::Blob::RenewBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = _detail::BlobRestClient::Blob::RenewLease(
          *(m_blobClient.Value().m_pipeline),
          m_blobClient.Value().m_blobUrl,
          protocolLayerOptions,
          context);

      Models::RenewLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::RenewLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_blobContainerClient.HasValue())
    {
      _detail::BlobRestClient::BlobContainer::RenewBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfMatch.HasValue(),
          "Blob container lease doesn't support If-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfNoneMatch.HasValue(),
          "Blob container lease doesn't support If-None-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.TagConditions.HasValue(),
          "Blob container lease doesn't support tag condition.");

      auto response = _detail::BlobRestClient::BlobContainer::RenewLease(
          *(m_blobContainerClient.Value().m_pipeline),
          m_blobContainerClient.Value().m_blobContainerUrl,
          protocolLayerOptions,
          context);

      Models::RenewLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::RenewLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  Azure::Response<Models::ReleaseLeaseResult> BlobLeaseClient::Release(
      const ReleaseLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    if (m_blobClient.HasValue())
    {
      _detail::BlobRestClient::Blob::ReleaseBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = _detail::BlobRestClient::Blob::ReleaseLease(
          *(m_blobClient.Value().m_pipeline),
          m_blobClient.Value().m_blobUrl,
          protocolLayerOptions,
          context);

      Models::ReleaseLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::ReleaseLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_blobContainerClient.HasValue())
    {
      _detail::BlobRestClient::BlobContainer::ReleaseBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfMatch.HasValue(),
          "Blob container lease doesn't support If-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfNoneMatch.HasValue(),
          "Blob container lease doesn't support If-None-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.TagConditions.HasValue(),
          "Blob container lease doesn't support tag condition.");

      auto response = _detail::BlobRestClient::BlobContainer::ReleaseLease(
          *(m_blobContainerClient.Value().m_pipeline),
          m_blobContainerClient.Value().m_blobContainerUrl,
          protocolLayerOptions,
          context);

      Models::ReleaseLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::ReleaseLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  Azure::Response<Models::ChangeLeaseResult> BlobLeaseClient::Change(
      const std::string& proposedLeaseId,
      const ChangeLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    if (m_blobClient.HasValue())
    {
      _detail::BlobRestClient::Blob::ChangeBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = _detail::BlobRestClient::Blob::ChangeLease(
          *(m_blobClient.Value().m_pipeline),
          m_blobClient.Value().m_blobUrl,
          protocolLayerOptions,
          context);

      {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_leaseId = response.Value.LeaseId;
      }

      Models::ChangeLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::ChangeLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_blobContainerClient.HasValue())
    {
      _detail::BlobRestClient::BlobContainer::ChangeBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfMatch.HasValue(),
          "Blob container lease doesn't support If-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfNoneMatch.HasValue(),
          "Blob container lease doesn't support If-None-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.TagConditions.HasValue(),
          "Blob container lease doesn't support tag condition.");

      auto response = _detail::BlobRestClient::BlobContainer::ChangeLease(
          *(m_blobContainerClient.Value().m_pipeline),
          m_blobContainerClient.Value().m_blobContainerUrl,
          protocolLayerOptions,
          context);

      {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_leaseId = response.Value.LeaseId;
      }

      Models::ChangeLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::ChangeLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  Azure::Response<Models::BreakLeaseResult> BlobLeaseClient::Break(
      const BreakLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    if (m_blobClient.HasValue())
    {
      _detail::BlobRestClient::Blob::BreakBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.BreakPeriod = options.BreakPeriod;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = _detail::BlobRestClient::Blob::BreakLease(
          *(m_blobClient.Value().m_pipeline),
          m_blobClient.Value().m_blobUrl,
          protocolLayerOptions,
          context);

      Models::BreakLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::BreakLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_blobContainerClient.HasValue())
    {
      _detail::BlobRestClient::BlobContainer::BreakBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.BreakPeriod = options.BreakPeriod;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfMatch.HasValue(),
          "Blob container lease doesn't support If-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.IfNoneMatch.HasValue(),
          "Blob container lease doesn't support If-None-Match condition.");
      AZURE_ASSERT_MSG(
          !options.AccessConditions.TagConditions.HasValue(),
          "Blob container lease doesn't support tag condition.");

      auto response = _detail::BlobRestClient::BlobContainer::BreakLease(
          *(m_blobContainerClient.Value().m_pipeline),
          m_blobContainerClient.Value().m_blobContainerUrl,
          protocolLayerOptions,
          context);

      Models::BreakLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::BreakLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }
}}} // namespace Azure::Storage::Blobs

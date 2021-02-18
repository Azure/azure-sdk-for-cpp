// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_lease_client.hpp"

#include <azure/core/uuid.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  const std::chrono::seconds BlobLeaseClient::InfiniteLeaseDuration{-1};

  std::string BlobLeaseClient::CreateUniqueLeaseId()
  {
    return Azure::Core::Uuid::CreateUuid().ToString();
  }

  Azure::Core::Response<Models::AcquireBlobLeaseResult> BlobLeaseClient::Acquire(
      std::chrono::seconds duration,
      const AcquireBlobLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    if (m_blobClient.HasValue())
    {
      Details::BlobRestClient::Blob::AcquireBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = m_leaseId;
      protocolLayerOptions.LeaseDuration = duration;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = Details::BlobRestClient::Blob::AcquireLease(
          context,
          *(m_blobClient.GetValue().m_pipeline),
          m_blobClient.GetValue().m_blobUrl,
          protocolLayerOptions);

      Models::AcquireBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::AcquireBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_blobContainerClient.HasValue())
    {
      Details::BlobRestClient::BlobContainer::AcquireBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = m_leaseId;
      protocolLayerOptions.LeaseDuration = duration;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      if (options.AccessConditions.IfMatch.HasValue()
          || options.AccessConditions.IfNoneMatch.HasValue()
          || options.AccessConditions.TagConditions.HasValue())
      {
        std::abort();
      }
      auto response = Details::BlobRestClient::BlobContainer::AcquireLease(
          context,
          *(m_blobContainerClient.GetValue().m_pipeline),
          m_blobContainerClient.GetValue().m_blobContainerUrl,
          protocolLayerOptions);

      Models::AcquireBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::AcquireBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::RenewBlobLeaseResult> BlobLeaseClient::Renew(
      const RenewBlobLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    if (m_blobClient.HasValue())
    {
      Details::BlobRestClient::Blob::RenewBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = Details::BlobRestClient::Blob::RenewLease(
          context,
          *(m_blobClient.GetValue().m_pipeline),
          m_blobClient.GetValue().m_blobUrl,
          protocolLayerOptions);

      Models::RenewBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::RenewBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_blobContainerClient.HasValue())
    {
      Details::BlobRestClient::BlobContainer::RenewBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      if (options.AccessConditions.IfMatch.HasValue()
          || options.AccessConditions.IfNoneMatch.HasValue()
          || options.AccessConditions.TagConditions.HasValue())
      {
        std::abort();
      }

      auto response = Details::BlobRestClient::BlobContainer::RenewLease(
          context,
          *(m_blobContainerClient.GetValue().m_pipeline),
          m_blobContainerClient.GetValue().m_blobContainerUrl,
          protocolLayerOptions);

      Models::RenewBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::RenewBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::ReleaseBlobLeaseResult> BlobLeaseClient::Release(
      const ReleaseBlobLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    if (m_blobClient.HasValue())
    {
      Details::BlobRestClient::Blob::ReleaseBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = Details::BlobRestClient::Blob::ReleaseLease(
          context,
          *(m_blobClient.GetValue().m_pipeline),
          m_blobClient.GetValue().m_blobUrl,
          protocolLayerOptions);

      Models::ReleaseBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::ReleaseBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_blobContainerClient.HasValue())
    {
      Details::BlobRestClient::BlobContainer::ReleaseBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      if (options.AccessConditions.IfMatch.HasValue()
          || options.AccessConditions.IfNoneMatch.HasValue()
          || options.AccessConditions.TagConditions.HasValue())
      {
        std::abort();
      }

      auto response = Details::BlobRestClient::BlobContainer::ReleaseLease(
          context,
          *(m_blobContainerClient.GetValue().m_pipeline),
          m_blobContainerClient.GetValue().m_blobContainerUrl,
          protocolLayerOptions);

      Models::ReleaseBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::ReleaseBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::ChangeBlobLeaseResult> BlobLeaseClient::Change(
      const std::string& proposedLeaseId,
      const ChangeBlobLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    if (m_blobClient.HasValue())
    {
      Details::BlobRestClient::Blob::ChangeBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = Details::BlobRestClient::Blob::ChangeLease(
          context,
          *(m_blobClient.GetValue().m_pipeline),
          m_blobClient.GetValue().m_blobUrl,
          protocolLayerOptions);

      Models::ChangeBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::ChangeBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_blobContainerClient.HasValue())
    {
      Details::BlobRestClient::BlobContainer::ChangeBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = m_leaseId;
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      if (options.AccessConditions.IfMatch.HasValue()
          || options.AccessConditions.IfNoneMatch.HasValue()
          || options.AccessConditions.TagConditions.HasValue())
      {
        std::abort();
      }

      auto response = Details::BlobRestClient::BlobContainer::ChangeLease(
          context,
          *(m_blobContainerClient.GetValue().m_pipeline),
          m_blobContainerClient.GetValue().m_blobContainerUrl,
          protocolLayerOptions);

      Models::ChangeBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::ChangeBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::BreakBlobLeaseResult> BlobLeaseClient::Break(
      const BreakBlobLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    if (m_blobClient.HasValue())
    {
      Details::BlobRestClient::Blob::BreakBlobLeaseOptions protocolLayerOptions;
      protocolLayerOptions.BreakPeriod = options.BreakPeriod;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
      protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
      protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
      protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

      auto response = Details::BlobRestClient::Blob::BreakLease(
          context,
          *(m_blobClient.GetValue().m_pipeline),
          m_blobClient.GetValue().m_blobUrl,
          protocolLayerOptions);

      Models::BreakBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::BreakBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_blobContainerClient.HasValue())
    {
      Details::BlobRestClient::BlobContainer::BreakBlobContainerLeaseOptions protocolLayerOptions;
      protocolLayerOptions.BreakPeriod = options.BreakPeriod;
      protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
      protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;

      if (options.AccessConditions.IfMatch.HasValue()
          || options.AccessConditions.IfNoneMatch.HasValue()
          || options.AccessConditions.TagConditions.HasValue())
      {
        std::abort();
      }

      auto response = Details::BlobRestClient::BlobContainer::BreakLease(
          context,
          *(m_blobContainerClient.GetValue().m_pipeline),
          m_blobContainerClient.GetValue().m_blobContainerUrl,
          protocolLayerOptions);

      Models::BreakBlobLeaseResult ret;
      ret.RequestId = std::move(response->RequestId);
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::BreakBlobLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }
}}} // namespace Azure::Storage::Blobs

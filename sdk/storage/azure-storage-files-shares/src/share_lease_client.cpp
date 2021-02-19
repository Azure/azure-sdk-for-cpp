// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_lease_client.hpp"

#include <azure/core/uuid.hpp>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  const std::chrono::seconds ShareLeaseClient::InfiniteLeaseDuration{-1};

  std::string ShareLeaseClient::CreateUniqueLeaseId()
  {
    return Azure::Core::Uuid::CreateUuid().ToString();
  }

  Azure::Core::Response<Models::AcquireShareLeaseResult> ShareLeaseClient::Acquire(
      std::chrono::seconds duration,
      const AcquireShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      Details::ShareRestClient::File::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = m_leaseId;
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = Details::ShareRestClient::File::AcquireLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::AcquireShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::AcquireShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      Details::ShareRestClient::Share::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = m_leaseId;
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = Details::ShareRestClient::Share::AcquireLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::AcquireShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::AcquireShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::RenewShareLeaseResult> ShareLeaseClient::Renew(
      const RenewShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      // Renew only support share level lease.
      std::abort();
    }
    else if (m_shareClient.HasValue())
    {
      Details::ShareRestClient::Share::RenewLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = Details::ShareRestClient::Share::RenewLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::RenewShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::RenewShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::ReleaseShareLeaseResult> ShareLeaseClient::Release(
      const ReleaseShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      Details::ShareRestClient::File::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = Details::ShareRestClient::File::ReleaseLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ReleaseShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::ReleaseShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      Details::ShareRestClient::Share::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = Details::ShareRestClient::Share::ReleaseLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ReleaseShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::ReleaseShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::ChangeShareLeaseResult> ShareLeaseClient::Change(
      const std::string& proposedLeaseId,
      const ChangeShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      Details::ShareRestClient::File::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = Details::ShareRestClient::File::ChangeLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ChangeShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::ChangeShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      Details::ShareRestClient::Share::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = Details::ShareRestClient::Share::ChangeLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ChangeShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Core::Response<Models::ChangeShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Core::Response<Models::BreakShareLeaseResult> ShareLeaseClient::Break(
      const BreakShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      Details::ShareRestClient::File::BreakLeaseOptions protocolLayerOptions;

      auto response = Details::ShareRestClient::File::BreakLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::BreakShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::BreakShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      Details::ShareRestClient::Share::BreakLeaseOptions protocolLayerOptions;

      auto response = Details::ShareRestClient::Share::BreakLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::BreakShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Core::Response<Models::BreakShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }
}}}} // namespace Azure::Storage::Files::Shares

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

  Azure::Response<Models::AcquireShareLeaseResult> ShareLeaseClient::Acquire(
      std::chrono::seconds duration,
      const AcquireShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::ShareRestClient::File::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = m_leaseId;
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = _detail::ShareRestClient::File::AcquireLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::AcquireShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Response<Models::AcquireShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = m_leaseId;
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = _detail::ShareRestClient::Share::AcquireLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::AcquireShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Response<Models::AcquireShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Response<Models::RenewShareLeaseResult> ShareLeaseClient::Renew(
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
      _detail::ShareRestClient::Share::RenewLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = _detail::ShareRestClient::Share::RenewLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::RenewShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Response<Models::RenewShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Response<Models::ReleaseShareLeaseResult> ShareLeaseClient::Release(
      const ReleaseShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::ShareRestClient::File::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = _detail::ShareRestClient::File::ReleaseLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ReleaseShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Response<Models::ReleaseShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;

      auto response = _detail::ShareRestClient::Share::ReleaseLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ReleaseShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Response<Models::ReleaseShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Response<Models::ChangeShareLeaseResult> ShareLeaseClient::Change(
      const std::string& proposedLeaseId,
      const ChangeShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::ShareRestClient::File::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = _detail::ShareRestClient::File::ChangeLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ChangeShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Response<Models::ChangeShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = m_leaseId;
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = _detail::ShareRestClient::Share::ChangeLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ChangeShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);
      ret.LeaseId = std::move(response->LeaseId);

      return Azure::Response<Models::ChangeShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }

  Azure::Response<Models::BreakShareLeaseResult> ShareLeaseClient::Break(
      const BreakShareLeaseOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::ShareRestClient::File::BreakLeaseOptions protocolLayerOptions;

      auto response = _detail::ShareRestClient::File::BreakLease(
          m_fileClient.GetValue().m_shareFileUrl,
          *(m_fileClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::BreakShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Response<Models::BreakShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::BreakLeaseOptions protocolLayerOptions;

      auto response = _detail::ShareRestClient::Share::BreakLease(
          m_shareClient.GetValue().m_shareUrl,
          *(m_shareClient.GetValue().m_pipeline),
          context,
          protocolLayerOptions);

      Models::BreakShareLeaseResult ret;
      ret.ETag = std::move(response->ETag);
      ret.LastModified = std::move(response->LastModified);

      return Azure::Response<Models::BreakShareLeaseResult>(
          std::move(ret), response.ExtractRawResponse());
    }
    else
    {
      std::abort();
    }
  }
}}}} // namespace Azure::Storage::Files::Shares

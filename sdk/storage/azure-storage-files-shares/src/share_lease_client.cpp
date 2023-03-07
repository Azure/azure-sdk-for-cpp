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

  Azure::Response<Models::AcquireLeaseResult> ShareLeaseClient::Acquire(
      std::chrono::seconds duration,
      const AcquireLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::FileClient::AcquireFileLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = GetLeaseId();
      protocolLayerOptions.Duration = static_cast<int32_t>(duration.count());
      protocolLayerOptions.AllowTrailingDot = m_fileClient.Value().m_allowTrailingDot;
      protocolLayerOptions.FileRequestIntent = m_fileClient.Value().m_shareTokenIntent;

      auto response = _detail::FileClient::AcquireLease(
          *(m_fileClient.Value().m_pipeline),
          m_fileClient.Value().m_shareFileUrl,
          protocolLayerOptions,
          context);

      Models::AcquireLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::AcquireLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareClient::AcquireShareLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseId = GetLeaseId();
      protocolLayerOptions.Duration = static_cast<int32_t>(duration.count());

      auto response = _detail::ShareClient::AcquireLease(
          *(m_shareClient.Value().m_pipeline),
          m_shareClient.Value().m_shareUrl,
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

  Azure::Response<Models::RenewLeaseResult> ShareLeaseClient::Renew(
      const RenewLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      AZURE_ASSERT_MSG(false, "File lease doesn't support renew");
      AZURE_NOT_IMPLEMENTED();
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareClient::RenewShareLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();

      auto response = _detail::ShareClient::RenewLease(
          *(m_shareClient.Value().m_pipeline),
          m_shareClient.Value().m_shareUrl,
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

  Azure::Response<Models::ReleaseLeaseResult> ShareLeaseClient::Release(
      const ReleaseLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::FileClient::ReleaseFileLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.AllowTrailingDot = m_fileClient.Value().m_allowTrailingDot;
      protocolLayerOptions.FileRequestIntent = m_fileClient.Value().m_shareTokenIntent;

      auto response = _detail::FileClient::ReleaseLease(
          *(m_fileClient.Value().m_pipeline),
          m_fileClient.Value().m_shareFileUrl,
          protocolLayerOptions,
          context);

      Models::ReleaseLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::ReleaseLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareClient::ReleaseShareLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();

      auto response = _detail::ShareClient::ReleaseLease(
          *(m_shareClient.Value().m_pipeline),
          m_shareClient.Value().m_shareUrl,
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

  Azure::Response<Models::ChangeLeaseResult> ShareLeaseClient::Change(
      const std::string& proposedLeaseId,
      const ChangeLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::FileClient::ChangeFileLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
      protocolLayerOptions.AllowTrailingDot = m_fileClient.Value().m_allowTrailingDot;
      protocolLayerOptions.FileRequestIntent = m_fileClient.Value().m_shareTokenIntent;

      auto response = _detail::FileClient::ChangeLease(
          *(m_fileClient.Value().m_pipeline),
          m_fileClient.Value().m_shareFileUrl,
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
    else if (m_shareClient.HasValue())
    {
      _detail::ShareClient::ChangeShareLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseId = GetLeaseId();
      protocolLayerOptions.ProposedLeaseId = proposedLeaseId;

      auto response = _detail::ShareClient::ChangeLease(
          *(m_shareClient.Value().m_pipeline),
          m_shareClient.Value().m_shareUrl,
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

  Azure::Response<Models::BreakLeaseResult> ShareLeaseClient::Break(
      const BreakLeaseOptions& options,
      const Azure::Core::Context& context)
  {
    (void)options;
    if (m_fileClient.HasValue())
    {
      _detail::FileClient::BreakFileLeaseOptions protocolLayerOptions;
      protocolLayerOptions.AllowTrailingDot = m_fileClient.Value().m_allowTrailingDot;
      protocolLayerOptions.FileRequestIntent = m_fileClient.Value().m_shareTokenIntent;

      auto response = _detail::FileClient::BreakLease(
          *(m_fileClient.Value().m_pipeline),
          m_fileClient.Value().m_shareFileUrl,
          protocolLayerOptions,
          context);

      Models::BreakLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::BreakLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareClient::BreakShareLeaseOptions protocolLayerOptions;

      auto response = _detail::ShareClient::BreakLease(

          *(m_shareClient.Value().m_pipeline),
          m_shareClient.Value().m_shareUrl,
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
}}}} // namespace Azure::Storage::Files::Shares

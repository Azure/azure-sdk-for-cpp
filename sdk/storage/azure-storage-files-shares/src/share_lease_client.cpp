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
      _detail::ShareRestClient::File::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = GetLeaseId();
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = _detail::ShareRestClient::File::AcquireLease(
          m_fileClient.Value().m_shareFileUrl,
          *(m_fileClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

      Models::AcquireLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);
      ret.LeaseId = std::move(response.Value.LeaseId);

      return Azure::Response<Models::AcquireLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::AcquireLeaseOptions protocolLayerOptions;
      protocolLayerOptions.ProposedLeaseIdOptional = GetLeaseId();
      protocolLayerOptions.LeaseDuration = static_cast<int32_t>(duration.count());

      auto response = _detail::ShareRestClient::Share::AcquireLease(
          m_shareClient.Value().m_shareUrl,
          *(m_shareClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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
      AZURE_ASSERT_MSG(false, "Share lease doesn't support renew");
      AZURE_NOT_IMPLEMENTED();
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::RenewLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = GetLeaseId();

      auto response = _detail::ShareRestClient::Share::RenewLease(
          m_shareClient.Value().m_shareUrl,
          *(m_shareClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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
      _detail::ShareRestClient::File::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = GetLeaseId();

      auto response = _detail::ShareRestClient::File::ReleaseLease(
          m_fileClient.Value().m_shareFileUrl,
          *(m_fileClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

      Models::ReleaseLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::ReleaseLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::ReleaseLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = GetLeaseId();

      auto response = _detail::ShareRestClient::Share::ReleaseLease(
          m_shareClient.Value().m_shareUrl,
          *(m_shareClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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
      _detail::ShareRestClient::File::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = GetLeaseId();
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = _detail::ShareRestClient::File::ChangeLease(
          m_fileClient.Value().m_shareFileUrl,
          *(m_fileClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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
      _detail::ShareRestClient::Share::ChangeLeaseOptions protocolLayerOptions;
      protocolLayerOptions.LeaseIdRequired = GetLeaseId();
      protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;

      auto response = _detail::ShareRestClient::Share::ChangeLease(
          m_shareClient.Value().m_shareUrl,
          *(m_shareClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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
      _detail::ShareRestClient::File::BreakLeaseOptions protocolLayerOptions;

      auto response = _detail::ShareRestClient::File::BreakLease(
          m_fileClient.Value().m_shareFileUrl,
          *(m_fileClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

      Models::BreakLeaseResult ret;
      ret.ETag = std::move(response.Value.ETag);
      ret.LastModified = std::move(response.Value.LastModified);

      return Azure::Response<Models::BreakLeaseResult>(
          std::move(ret), std::move(response.RawResponse));
    }
    else if (m_shareClient.HasValue())
    {
      _detail::ShareRestClient::Share::BreakLeaseOptions protocolLayerOptions;

      auto response = _detail::ShareRestClient::Share::BreakLease(
          m_shareClient.Value().m_shareUrl,
          *(m_shareClient.Value().m_pipeline),
          context,
          protocolLayerOptions);

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

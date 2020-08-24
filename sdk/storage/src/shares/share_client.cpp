// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "shares/share_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/constants.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/storage_version.hpp"
#include "azure/core/credentials/policy/policies.hpp"
#include "azure/core/http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareClient ShareClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto shareUri = std::move(parsedConnectionString.FileServiceUri);
    shareUri.AppendPath(shareName, true);

    if (parsedConnectionString.KeyCredential)
    {
      return ShareClient(shareUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareClient(shareUri.ToString(), options);
    }
  }

  ShareClient::ShareClient(
      const std::string& shareUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient::ShareClient(
      const std::string& shareUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(
        std::make_unique<Core::Credentials::Policy::BearerTokenAuthenticationPolicy>(
            credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient::ShareClient(const std::string& shareUri, const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<CreateShareResult> ShareClient::Create(
      const CreateShareOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.ShareQuota = options.ShareQuota;
    return ShareRestClient::Share::Create(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<DeleteShareResult> ShareClient::Delete(
      const DeleteShareOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::DeleteOptions();
    protocolLayerOptions.ShareSnapshot = options.ShareSnapshot;
    if (options.IncludeSnapshots.HasValue() and options.IncludeSnapshots.GetValue())
    {
      protocolLayerOptions.XMsDeleteSnapshots = DeleteSnapshotsOptionType::Include;
    }
    return ShareRestClient::Share::Delete(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      const CreateShareSnapshotOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreateSnapshotOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    return ShareRestClient::Share::CreateSnapshot(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetSharePropertiesResult> ShareClient::GetProperties(
      const GetSharePropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetPropertiesOptions();
    protocolLayerOptions.ShareSnapshot = options.ShareSnapshot;
    return ShareRestClient::Share::GetProperties(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareQuotaResult> ShareClient::SetQuota(
      int32_t quota,
      const SetShareQuotaOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetQuotaOptions();
    protocolLayerOptions.ShareQuota = quota;
    return ShareRestClient::Share::SetQuota(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareMetadataResult> ShareClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetShareMetadataOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return ShareRestClient::Share::SetMetadata(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetShareAccessPolicyResult> ShareClient::GetAccessPolicy(
      const GetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetAccessPolicyOptions();
    return ShareRestClient::Share::GetAccessPolicy(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      const std::vector<SignedIdentifier>& accessPolicy,
      const SetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetAccessPolicyOptions();
    protocolLayerOptions.ShareAcl = accessPolicy;
    return ShareRestClient::Share::SetAccessPolicy(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetShareStatisticsResult> ShareClient::GetStatistics(
      const GetShareStatsOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetStatisticsOptions();
    return ShareRestClient::Share::GetStatistics(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<CreateSharePermissionResult> ShareClient::CreatePermission(
      const std::string& permission,
      const CreateSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreatePermissionOptions();
    protocolLayerOptions.Permission.Permission = permission;
    return ShareRestClient::Share::CreatePermission(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetSharePermissionResult> ShareClient::GetPermission(
      const std::string& permissionKey,
      const GetSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetPermissionOptions();
    protocolLayerOptions.FilePermissionKeyRequired = permissionKey;
    return ShareRestClient::Share::GetPermission(
        m_shareUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

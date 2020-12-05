// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareClient ShareClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto shareUri = std::move(parsedConnectionString.FileServiceUri);
    shareUri.AppendPath(Storage::Details::UrlEncodePath(shareName));

    if (parsedConnectionString.KeyCredential)
    {
      return ShareClient(shareUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareClient(shareUri.GetAbsoluteUrl(), options);
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
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient::ShareClient(
      const std::string& shareUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
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
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient ShareClient::GetDirectoryClient(const std::string& directoryPath) const
  {
    auto builder = m_shareUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(directoryPath));
    return DirectoryClient(builder, m_pipeline);
  }

  FileClient ShareClient::GetFileClient(const std::string& filePath) const
  {
    auto builder = m_shareUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(filePath));
    return FileClient(builder, m_pipeline);
  }

  ShareClient ShareClient::WithSnapshot(const std::string& snapshot) const
  {
    ShareClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_shareUri.RemoveQueryParameter(Details::c_ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareUri.AppendQueryParameter(
          Details::c_ShareSnapshotQueryParameter,
          Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  Azure::Core::Response<CreateShareResult> ShareClient::Create(
      const CreateShareOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.ShareQuota = options.ShareQuota;
    return ShareRestClient::Share::Create(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<DeleteShareResult> ShareClient::Delete(
      const DeleteShareOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::DeleteOptions();
    if (options.IncludeSnapshots.HasValue() and options.IncludeSnapshots.GetValue())
    {
      protocolLayerOptions.XMsDeleteSnapshots = DeleteSnapshotsOptionType::Include;
    }
    return ShareRestClient::Share::Delete(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      const CreateShareSnapshotOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreateSnapshotOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    return ShareRestClient::Share::CreateSnapshot(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetSharePropertiesResult> ShareClient::GetProperties(
      const GetSharePropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetPropertiesOptions();
    return ShareRestClient::Share::GetProperties(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareQuotaResult> ShareClient::SetQuota(
      int32_t quota,
      const SetShareQuotaOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetQuotaOptions();
    protocolLayerOptions.ShareQuota = quota;
    return ShareRestClient::Share::SetQuota(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareMetadataResult> ShareClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetShareMetadataOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return ShareRestClient::Share::SetMetadata(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetShareAccessPolicyResult> ShareClient::GetAccessPolicy(
      const GetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetAccessPolicyOptions();
    return ShareRestClient::Share::GetAccessPolicy(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      const std::vector<SignedIdentifier>& accessPolicy,
      const SetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::SetAccessPolicyOptions();
    protocolLayerOptions.ShareAcl = accessPolicy;
    return ShareRestClient::Share::SetAccessPolicy(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetShareStatisticsResult> ShareClient::GetStatistics(
      const GetShareStatsOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetStatisticsOptions();
    return ShareRestClient::Share::GetStatistics(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<CreateSharePermissionResult> ShareClient::CreatePermission(
      const std::string& permission,
      const CreateSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::CreatePermissionOptions();
    protocolLayerOptions.Permission.Permission = permission;
    return ShareRestClient::Share::CreatePermission(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetSharePermissionResult> ShareClient::GetPermission(
      const std::string& permissionKey,
      const GetSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Share::GetPermissionOptions();
    protocolLayerOptions.FilePermissionKeyRequired = permissionKey;
    return ShareRestClient::Share::GetPermission(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<ListFilesAndDirectoriesSegmentResult>
  ShareClient::ListFilesAndDirectoriesSegment(
      const ListFilesAndDirectoriesSegmentOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::ListFilesAndDirectoriesSegmentOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    auto result = ShareRestClient::Directory::ListFilesAndDirectoriesSegment(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
    ListFilesAndDirectoriesSegmentResult ret;
    ret.ServiceEndpoint = std::move(result->ServiceEndpoint);
    ret.ShareName = std::move(result->ShareName);
    ret.ShareSnapshot = std::move(result->ShareSnapshot);
    ret.DirectoryPath = std::move(result->DirectoryPath);
    ret.Prefix = std::move(result->Prefix);
    ret.PreviousContinuationToken = std::move(result->PreviousContinuationToken);
    ret.MaxResults = result->MaxResults;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.DirectoryItems = std::move(result->Segment.DirectoryItems);
    ret.FileItems = std::move(result->Segment.FileItems);

    return Azure::Core::Response<ListFilesAndDirectoriesSegmentResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<AcquireShareLeaseResult> ShareClient::AcquireLease(
      const std::string& proposedLeaseId,
      int32_t duration,
      const AcquireShareLeaseOptions& options) const
  {
    ShareRestClient::Share::AcquireLeaseOptions protocolLayerOptions;
    protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;
    protocolLayerOptions.LeaseDuration = duration;
    return ShareRestClient::Share::AcquireLease(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<ChangeShareLeaseResult> ShareClient::ChangeLease(
      const std::string& leaseId,
      const std::string& proposedLeaseId,
      const ChangeShareLeaseOptions& options) const
  {
    ShareRestClient::Share::ChangeLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdRequired = leaseId;
    protocolLayerOptions.ProposedLeaseIdOptional = proposedLeaseId;
    return ShareRestClient::Share::ChangeLease(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<ReleaseShareLeaseResult> ShareClient::ReleaseLease(
      const std::string& leaseId,
      const ReleaseShareLeaseOptions& options) const
  {
    ShareRestClient::Share::ReleaseLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdRequired = leaseId;
    return ShareRestClient::Share::ReleaseLease(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<BreakShareLeaseResult> ShareClient::BreakLease(
      const BreakShareLeaseOptions& options) const
  {
    ShareRestClient::Share::BreakLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseBreakPeriod = options.BreakPeriod;
    return ShareRestClient::Share::BreakLease(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<RenewShareLeaseResult> ShareClient::RenewLease(
      const std::string& leaseId,
      const RenewShareLeaseOptions& options) const
  {
    ShareRestClient::Share::RenewLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseIdRequired = leaseId;
    return ShareRestClient::Share::RenewLease(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_client.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareClient ShareClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto shareUrl = std::move(parsedConnectionString.FileServiceUrl);
    shareUrl.AppendPath(_internal::UrlEncodePath(shareName));

    if (parsedConnectionString.KeyCredential)
    {
      return ShareClient(shareUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareClient(shareUrl.GetAbsoluteUrl(), options);
    }
  }

  ShareClient::ShareClient(
      const std::string& shareUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareUrl(shareUrl)
  {
    ShareClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareClient::ShareClient(const std::string& shareUrl, const ShareClientOptions& options)
      : m_shareUrl(shareUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::FileServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareDirectoryClient ShareClient::GetRootDirectoryClient() const
  {
    return ShareDirectoryClient(m_shareUrl, m_pipeline);
  }

  ShareClient ShareClient::WithSnapshot(const std::string& snapshot) const
  {
    ShareClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_shareUrl.RemoveQueryParameter(_detail::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareUrl.AppendQueryParameter(
          _detail::ShareSnapshotQueryParameter, _internal::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  Azure::Response<Models::CreateShareResult> ShareClient::Create(
      const CreateShareOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareClient::CreateShareOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    protocolLayerOptions.Quota = options.ShareQuotaInGiB;
    protocolLayerOptions.AccessTier = options.AccessTier;
    protocolLayerOptions.EnabledProtocols = options.EnabledProtocols;
    protocolLayerOptions.RootSquash = options.RootSquash;
    auto result
        = _detail::ShareClient::Create(*m_pipeline, m_shareUrl, protocolLayerOptions, context);
    Models::CreateShareResult ret;
    ret.Created = true;
    ret.ETag = std::move(result.Value.ETag);
    ret.LastModified = std::move(result.Value.LastModified);
    return Azure::Response<Models::CreateShareResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::CreateShareResult> ShareClient::CreateIfNotExists(
      const CreateShareOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ShareAlreadyExists)
      {
        Models::CreateShareResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreateShareResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteShareResult> ShareClient::Delete(
      const DeleteShareOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareClient::DeleteShareOptions();
    if (options.DeleteSnapshots.HasValue() && options.DeleteSnapshots.Value())
    {
      protocolLayerOptions.DeleteSnapshots = Models::DeleteSnapshotsOption::Include;
    }
    auto result
        = _detail::ShareClient::Delete(*m_pipeline, m_shareUrl, protocolLayerOptions, context);
    Models::DeleteShareResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteShareResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteShareResult> ShareClient::DeleteIfExists(
      const DeleteShareOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ShareNotFound)
      {
        Models::DeleteShareResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteShareResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      const CreateShareSnapshotOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareClient::CreateShareSnapshotOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    return _detail::ShareClient::CreateSnapshot(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ShareProperties> ShareClient::GetProperties(
      const GetSharePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::GetSharePropertiesOptions();
    return _detail::ShareClient::GetProperties(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetSharePropertiesResult> ShareClient::SetProperties(
      const SetSharePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareClient::SetSharePropertiesOptions();
    protocolLayerOptions.Quota = options.ShareQuotaInGiB;
    protocolLayerOptions.AccessTier = options.AccessTier;
    protocolLayerOptions.RootSquash = options.RootSquash;
    return _detail::ShareClient::SetProperties(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetShareMetadataResult> ShareClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::SetShareMetadataOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    return _detail::ShareClient::SetMetadata(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ShareAccessPolicy> ShareClient::GetAccessPolicy(
      const GetShareAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::GetShareAccessPolicyOptions();
    return _detail::ShareClient::GetAccessPolicy(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      const std::vector<Models::SignedIdentifier>& accessPolicy,
      const SetShareAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::SetShareAccessPolicyOptions();
    protocolLayerOptions.ShareAcl = accessPolicy;
    return _detail::ShareClient::SetAccessPolicy(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ShareStatistics> ShareClient::GetStatistics(
      const GetShareStatisticsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::GetShareStatisticsOptions();
    return _detail::ShareClient::GetStatistics(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CreateSharePermissionResult> ShareClient::CreatePermission(
      const std::string& permission,
      const CreateSharePermissionOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::CreateSharePermissionOptions();
    protocolLayerOptions.SharePermission.Permission = permission;
    return _detail::ShareClient::CreatePermission(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);
  }

  Azure::Response<std::string> ShareClient::GetPermission(
      const std::string& permissionKey,
      const GetSharePermissionOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareClient::GetSharePermissionOptions();
    protocolLayerOptions.FilePermissionKey = permissionKey;
    auto result = _detail::ShareClient::GetPermission(
        *m_pipeline, m_shareUrl, protocolLayerOptions, context);

    return Azure::Response<std::string>(result.Value.Permission, std::move(result.RawResponse));
  }

}}}} // namespace Azure::Storage::Files::Shares
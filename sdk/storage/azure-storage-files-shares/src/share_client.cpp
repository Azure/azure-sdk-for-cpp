// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

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
    auto shareUri = std::move(parsedConnectionString.FileServiceUrl);
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
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::FileServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient::ShareClient(const std::string& shareUri, const ShareClientOptions& options)
      : m_shareUri(shareUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::FileServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareDirectoryClient ShareClient::GetRootDirectoryClient() const
  {
    return GetDirectoryClient("");
  }

  ShareDirectoryClient ShareClient::GetDirectoryClient(const std::string& directoryPath) const
  {
    auto builder = m_shareUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(directoryPath));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareClient::GetFileClient(const std::string& filePath) const
  {
    auto builder = m_shareUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(filePath));
    return ShareFileClient(builder, m_pipeline);
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

  Azure::Core::Response<Models::CreateShareResult> ShareClient::Create(
      const CreateShareOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.ShareQuota = options.ShareQuotaInGiB;
    auto result = Details::ShareRestClient::Share::Create(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::CreateShareResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<Models::CreateShareResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateShareResult> ShareClient::CreateIfNotExists(
      const CreateShareOptions& options) const
  {
    try
    {
      return Create(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareAlreadyExists)
      {
        Models::CreateShareResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateShareResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteShareResult> ShareClient::Delete(
      const DeleteShareOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::DeleteOptions();
    if (options.IncludeSnapshots.HasValue() && options.IncludeSnapshots.GetValue())
    {
      protocolLayerOptions.XMsDeleteSnapshots = Models::DeleteSnapshotsOptionType::Include;
    }
    auto result = Details::ShareRestClient::Share::Delete(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::DeleteShareResult ret;
    ret.Deleted = true;
    return Azure::Core::Response<Models::DeleteShareResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareResult> ShareClient::DeleteIfExists(
      const DeleteShareOptions& options) const
  {
    try
    {
      return Delete(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareNotFound)
      {
        Models::DeleteShareResult ret;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteShareResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      const CreateShareSnapshotOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreateSnapshotOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    return Details::ShareRestClient::Share::CreateSnapshot(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetSharePropertiesResult> ShareClient::GetProperties(
      const GetSharePropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetPropertiesOptions();
    return Details::ShareRestClient::Share::GetProperties(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareQuotaResult> ShareClient::SetQuota(
      int32_t quotaInGiB,
      const SetShareQuotaOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetQuotaOptions();
    protocolLayerOptions.ShareQuota = quotaInGiB;
    return Details::ShareRestClient::Share::SetQuota(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareMetadataResult> ShareClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareMetadataOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return Details::ShareRestClient::Share::SetMetadata(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetShareAccessPolicyResult> ShareClient::GetAccessPolicy(
      const GetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetAccessPolicyOptions();
    return Details::ShareRestClient::Share::GetAccessPolicy(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      const std::vector<Models::SignedIdentifier>& accessPolicy,
      const SetShareAccessPolicyOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetAccessPolicyOptions();
    protocolLayerOptions.ShareAcl = accessPolicy;
    return Details::ShareRestClient::Share::SetAccessPolicy(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetShareStatisticsResult> ShareClient::GetStatistics(
      const GetShareStatsOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetStatisticsOptions();
    return Details::ShareRestClient::Share::GetStatistics(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateSharePermissionResult> ShareClient::CreatePermission(
      const std::string& permission,
      const CreateSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreatePermissionOptions();
    protocolLayerOptions.Permission.Permission = permission;
    return Details::ShareRestClient::Share::CreatePermission(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetSharePermissionResult> ShareClient::GetPermission(
      const std::string& permissionKey,
      const GetSharePermissionOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetPermissionOptions();
    protocolLayerOptions.FilePermissionKeyRequired = permissionKey;
    return Details::ShareRestClient::Share::GetPermission(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>
  ShareClient::ListFilesAndDirectoriesSinglePage(
      const ListFilesAndDirectoriesSinglePageOptions& options) const
  {
    auto protocolLayerOptions
        = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::ListFilesAndDirectoriesSinglePageResult ret;
    ret.ServiceEndpoint = std::move(result->ServiceEndpoint);
    ret.ShareName = std::move(result->ShareName);
    ret.ShareSnapshot = std::move(result->ShareSnapshot);
    ret.DirectoryPath = std::move(result->DirectoryPath);
    ret.Prefix = std::move(result->Prefix);
    ret.PreviousContinuationToken = std::move(result->PreviousContinuationToken);
    ret.PageSizeHint = result->PageSizeHint;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.DirectoryItems = std::move(result->SinglePage.DirectoryItems);
    ret.FileItems = std::move(result->SinglePage.FileItems);

    return Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::Shares

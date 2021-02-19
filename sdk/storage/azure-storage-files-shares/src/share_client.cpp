// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

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
    auto shareUrl = std::move(parsedConnectionString.FileServiceUrl);
    shareUrl.AppendPath(Storage::Details::UrlEncodePath(shareName));

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
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Storage::Details::SharedKeyPolicy>(credential),
            options));
  }

  ShareClient::ShareClient(const std::string& shareUrl, const ShareClientOptions& options)
      : m_shareUrl(shareUrl)
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            nullptr,
            options));
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
      newClient.m_shareUrl.RemoveQueryParameter(Details::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareUrl.AppendQueryParameter(
          Details::ShareSnapshotQueryParameter,
          Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  Azure::Core::Response<Models::CreateShareResult> ShareClient::Create(
      const CreateShareOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.ShareQuota = options.ShareQuotaInGiB;
    protocolLayerOptions.XMsAccessTier = options.AccessTier;
    auto result = Details::ShareRestClient::Share::Create(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateShareResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::CreateShareResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateShareResult> ShareClient::CreateIfNotExists(
      const CreateShareOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareAlreadyExists)
      {
        Models::CreateShareResult ret;
        ret.Created = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Core::Response<Models::CreateShareResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteShareResult> ShareClient::Delete(
      const DeleteShareOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::DeleteOptions();
    if (options.DeleteSnapshots.HasValue() && options.DeleteSnapshots.GetValue())
    {
      protocolLayerOptions.XMsDeleteSnapshots = Models::DeleteSnapshotsOptionType::Include;
    }
    auto result = Details::ShareRestClient::Share::Delete(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteShareResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteShareResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareResult> ShareClient::DeleteIfExists(
      const DeleteShareOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareNotFound)
      {
        Models::DeleteShareResult ret;
        ret.Deleted = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Core::Response<Models::DeleteShareResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      const CreateShareSnapshotOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreateSnapshotOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    return Details::ShareRestClient::Share::CreateSnapshot(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetSharePropertiesResult> ShareClient::GetProperties(
      const GetSharePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetPropertiesOptions();
    return Details::ShareRestClient::Share::GetProperties(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetSharePropertiesResult> ShareClient::SetProperties(
      const SetSharePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetPropertiesOptions();
    protocolLayerOptions.ShareQuota = options.ShareQuotaInGiB;
    protocolLayerOptions.XMsAccessTier = options.AccessTier;
    return Details::ShareRestClient::Share::SetProperties(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareMetadataResult> ShareClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return Details::ShareRestClient::Share::SetMetadata(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetShareAccessPolicyResult> ShareClient::GetAccessPolicy(
      const GetShareAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetAccessPolicyOptions();
    return Details::ShareRestClient::Share::GetAccessPolicy(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      const std::vector<Models::SignedIdentifier>& accessPolicy,
      const SetShareAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::SetAccessPolicyOptions();
    protocolLayerOptions.ShareAcl = accessPolicy;
    return Details::ShareRestClient::Share::SetAccessPolicy(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetShareStatisticsResult> ShareClient::GetStatistics(
      const GetShareStatsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetStatisticsOptions();
    return Details::ShareRestClient::Share::GetStatistics(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateSharePermissionResult> ShareClient::CreatePermission(
      const std::string& permission,
      const CreateSharePermissionOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::CreatePermissionOptions();
    protocolLayerOptions.Permission.FilePermission = permission;
    return Details::ShareRestClient::Share::CreatePermission(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetSharePermissionResult> ShareClient::GetPermission(
      const std::string& permissionKey,
      const GetSharePermissionOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Share::GetPermissionOptions();
    protocolLayerOptions.FilePermissionKeyRequired = permissionKey;
    return Details::ShareRestClient::Share::GetPermission(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>
  ShareClient::ListFilesAndDirectoriesSinglePage(
      const ListFilesAndDirectoriesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions
        = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ListFilesAndDirectoriesSinglePageResult ret;
    ret.ServiceEndpoint = std::move(result->ServiceEndpoint);
    ret.ShareName = std::move(result->ShareName);
    ret.ShareSnapshot = std::move(result->ShareSnapshot);
    ret.DirectoryPath = std::move(result->DirectoryPath);
    ret.Prefix = std::move(result->Prefix);
    ret.PageSizeHint = result->PageSizeHint;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.DirectoryItems = std::move(result->SinglePage.DirectoryItems);
    ret.FileItems = std::move(result->SinglePage.FileItems);
    ret.RequestId = std::move(result->RequestId);

    return Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::Shares

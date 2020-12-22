// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_directory_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareDirectoryClient ShareDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& directoryPath,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUri = std::move(parsedConnectionString.FileServiceUrl);
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(shareName));
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(directoryPath));

    if (parsedConnectionString.KeyCredential)
    {
      return ShareDirectoryClient(
          directoryUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareDirectoryClient(directoryUri.GetAbsoluteUrl(), options);
    }
  }

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::FileServicePackageName, Version::VersionString()));
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

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUri,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::FileServicePackageName, Version::VersionString()));
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

  ShareDirectoryClient ShareDirectoryClient::GetSubShareDirectoryClient(
      const std::string& subDirectoryName) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(subDirectoryName));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareDirectoryClient::GetShareFileClient(const std::string& filePath) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(filePath));
    return ShareFileClient(builder, m_pipeline);
  }

  ShareDirectoryClient ShareDirectoryClient::WithShareSnapshot(
      const std::string& shareSnapshot) const
  {
    ShareDirectoryClient newClient(*this);
    if (shareSnapshot.empty())
    {
      newClient.m_shareDirectoryUri.RemoveQueryParameter(Details::c_ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareDirectoryUri.AppendQueryParameter(
          Details::c_ShareSnapshotQueryParameter,
          Storage::Details::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Core::Response<Models::CreateDirectoryResult> ShareDirectoryClient::Create(
      const CreateDirectoryOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes
        = Details::FileAttributesToString(options.SmbProperties.Attributes);
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes
          = Details::FileAttributesToString(Models::FileAttributes::Directory);
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.DirectoryPermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.DirectoryPermission.GetValue();
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(c_FileInheritPermission);
    }
    return Details::ShareRestClient::Directory::Create(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteDirectoryOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::DeleteOptions();
    return Details::ShareRestClient::Directory::Delete(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetDirectoryPropertiesResult> ShareDirectoryClient::GetProperties(
      const GetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::GetPropertiesOptions();
    return Details::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDirectoryPropertiesResult> ShareDirectoryClient::SetProperties(
      Models::FileShareSmbProperties smbProperties,
      const SetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = Details::FileAttributesToString(smbProperties.Attributes);
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.GetValue().GetRfc3339String(
          Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FileDefaultTimeValue);
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = smbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission.GetValue();
    }
    else if (smbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = smbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(c_FileInheritPermission);
    }
    return Details::ShareRestClient::Directory::SetProperties(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDirectoryMetadataOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    return Details::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSegmentResult>
  ShareDirectoryClient::ListFilesAndDirectoriesSegment(
      const ListFilesAndDirectoriesSegmentOptions& options) const
  {
    auto protocolLayerOptions
        = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSegmentOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    auto result = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSegment(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::ListFilesAndDirectoriesSegmentResult ret;
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

    return Azure::Core::Response<Models::ListFilesAndDirectoriesSegmentResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ListDirectoryHandlesSegmentResult>
  ShareDirectoryClient::ListHandlesSegment(const ListDirectoryHandlesSegmentOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = Details::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::ListDirectoryHandlesSegmentResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Handles = std::move(result->HandleList);

    return Azure::Core::Response<Models::ListDirectoryHandlesSegmentResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseDirectoryHandleResult>
  ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseDirectoryHandleOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = Details::ShareRestClient::File::ForceCloseHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    return Azure::Core::Response<Models::ForceCloseDirectoryHandleResult>(
        Models::ForceCloseDirectoryHandleResult(), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseAllDirectoryHandlesResult>
  ShareDirectoryClient::ForceCloseAllHandles(
      const ForceCloseAllDirectoryHandlesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = c_FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    return Details::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

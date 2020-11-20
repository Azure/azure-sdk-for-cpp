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

  DirectoryClient DirectoryClient::CreateFromConnectionString(
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
      return DirectoryClient(
          directoryUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DirectoryClient(directoryUri.GetAbsoluteUrl(), options);
    }
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient DirectoryClient::GetSubDirectoryClient(const std::string& subDirectoryName) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(subDirectoryName));
    return DirectoryClient(builder, m_pipeline);
  }

  FileClient DirectoryClient::GetFileClient(const std::string& filePath) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(filePath));
    return FileClient(builder, m_pipeline);
  }

  DirectoryClient DirectoryClient::WithShareSnapshot(const std::string& shareSnapshot) const
  {
    DirectoryClient newClient(*this);
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

  Azure::Core::Response<Models::CreateDirectoryResult> DirectoryClient::Create(
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
    if (options.SmbProperties.CreationTime.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreationTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWriteTime.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWriteTime.GetValue();
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

  Azure::Core::Response<Models::DeleteDirectoryResult> DirectoryClient::Delete(
      const DeleteDirectoryOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::DeleteOptions();
    return Details::ShareRestClient::Directory::Delete(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetDirectoryPropertiesResult> DirectoryClient::GetProperties(
      const GetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::GetPropertiesOptions();
    return Details::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDirectoryPropertiesResult> DirectoryClient::SetProperties(
      Models::FileShareSmbProperties smbProperties,
      const SetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = Details::FileAttributesToString(smbProperties.Attributes);
    if (smbProperties.CreationTime.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreationTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FilePreserveSmbProperties);
    }
    if (smbProperties.LastWriteTime.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = smbProperties.LastWriteTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(c_FilePreserveSmbProperties);
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

  Azure::Core::Response<Models::SetDirectoryMetadataResult> DirectoryClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const SetDirectoryMetadataOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return Details::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSegmentResult>
  DirectoryClient::ListFilesAndDirectoriesSegment(
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
  DirectoryClient::ListHandlesSegment(const ListDirectoryHandlesSegmentOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = Details::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::ListDirectoryHandlesSegmentResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.HandleList = std::move(result->HandleList);

    return Azure::Core::Response<Models::ListDirectoryHandlesSegmentResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseDirectoryHandleResult> DirectoryClient::ForceCloseHandle(
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
  DirectoryClient::ForceCloseAllHandles(const ForceCloseAllDirectoryHandlesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = c_FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    return Details::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

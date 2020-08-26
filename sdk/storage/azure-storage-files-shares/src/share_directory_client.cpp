// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_directory_client.hpp"

#include "azure/core/credentials/policy/policies.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_version.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  DirectoryClient DirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& directoryPath,
      const DirectoryClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUri = std::move(parsedConnectionString.FileServiceUri);
    directoryUri.AppendPath(shareName, true);
    directoryUri.AppendPath(directoryPath, true);

    if (parsedConnectionString.KeyCredential)
    {
      return DirectoryClient(
          directoryUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DirectoryClient(directoryUri.ToString(), options);
    }
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const DirectoryClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
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
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const DirectoryClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
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
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Core::Credentials::Policy::BearerTokenAuthenticationPolicy>(
            credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& shareDirectoryUri,
      const DirectoryClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
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
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient DirectoryClient::GetSubDirectoryClient(const std::string& subDirectoryName) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(subDirectoryName, true);
    return DirectoryClient(builder, m_pipeline);
  }

  FileClient DirectoryClient::GetFileClient(const std::string& filePath) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(filePath, true);
    return FileClient(builder, m_pipeline);
  }

  DirectoryClient DirectoryClient::WithSnapshot(const std::string& snapshot) const
  {
    DirectoryClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_shareDirectoryUri.RemoveQuery(Details::c_ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareDirectoryUri.AppendQuery(Details::c_ShareSnapshotQueryParameter, snapshot);
    }
    return newClient;
  }

  Azure::Core::Response<CreateDirectoryResult> DirectoryClient::Create(
      const CreateDirectoryOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes = FileAttributesToString(options.SmbProperties.Attributes);
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = FileAttributesToString(FileAttributes::Directory);
    }
    if (options.SmbProperties.FileCreationTime.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.FileCreationTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.SmbProperties.FileLastWriteTime.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.FileLastWriteTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(c_FileDefaultTimeValue);
    }
    if (options.DirectoryPermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.DirectoryPermission.GetValue();
    }
    else if (options.SmbProperties.FilePermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.FilePermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(c_FileInheritPermission);
    }
    return ShareRestClient::Directory::Create(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<DeleteDirectoryResult> DirectoryClient::Delete(
      const DeleteDirectoryOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::DeleteOptions();
    return ShareRestClient::Directory::Delete(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetDirectoryPropertiesResult> DirectoryClient::GetProperties(
      const GetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::GetPropertiesOptions();
    return ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetDirectoryPropertiesResult> DirectoryClient::SetProperties(
      FileShareSmbProperties smbProperties,
      const SetDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = FileAttributesToString(smbProperties.Attributes);
    if (smbProperties.FileCreationTime.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.FileCreationTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(c_FilePreserveSmbProperties);
    }
    if (smbProperties.FileLastWriteTime.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = smbProperties.FileLastWriteTime.GetValue();
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(c_FilePreserveSmbProperties);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission.GetValue();
    }
    else if (smbProperties.FilePermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = smbProperties.FilePermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(c_FileInheritPermission);
    }
    return ShareRestClient::Directory::SetProperties(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetDirectoryMetadataResult> DirectoryClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const SetDirectoryMetadataOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = metadata;
    return ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<ListFilesAndDirectoriesSegmentedResult>
  DirectoryClient::ListFilesAndDirectoriesSegmented(
      const ListFilesAndDirectoriesSegmentedOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::ListFilesAndDirectoriesSegmentOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.MaxResults = options.MaxResults;
    auto result = ShareRestClient::Directory::ListFilesAndDirectoriesSegment(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    ListFilesAndDirectoriesSegmentedResult ret;
    ret.ServiceEndpoint = std::move(result->ServiceEndpoint);
    ret.ShareName = std::move(result->ShareName);
    ret.ShareSnapshot = std::move(result->ShareSnapshot);
    ret.DirectoryPath = std::move(result->DirectoryPath);
    ret.Prefix = std::move(result->Prefix);
    ret.Marker = std::move(result->Marker);
    ret.MaxResults = result->MaxResults;
    ret.NextMarker = std::move(result->NextMarker);
    ret.DirectoryItems = std::move(result->Segment.DirectoryItems);
    ret.FileItems = std::move(result->Segment.FileItems);

    return Azure::Core::Response<ListFilesAndDirectoriesSegmentedResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<ListDirectoryHandlesSegmentedResult> DirectoryClient::ListHandlesSegmented(
      const ListDirectoryHandlesSegmentedOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    ListDirectoryHandlesSegmentedResult ret;
    ret.NextMarker = std::move(result->NextMarker);
    ret.HandleList = std::move(result->HandleList);

    return Azure::Core::Response<ListDirectoryHandlesSegmentedResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<ForceCloseDirectoryHandlesResult> DirectoryClient::ForceCloseHandles(
      const std::string& handleId,
      const ForceCloseDirectoryHandlesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.Recursive = options.Recursive;
    return ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

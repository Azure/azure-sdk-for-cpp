// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_directory_client.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/shares/share_file_client.hpp"

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareDirectoryClient ShareDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& directoryName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto directoryUrl = std::move(parsedConnectionString.FileServiceUrl);
    directoryUrl.AppendPath(_internal::UrlEncodePath(shareName));
    directoryUrl.AppendPath(_internal::UrlEncodePath(directoryName));

    if (parsedConnectionString.KeyCredential)
    {
      return ShareDirectoryClient(
          directoryUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareDirectoryClient(directoryUrl.GetAbsoluteUrl(), options);
    }
  }

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const ShareClientOptions& options)
      : m_shareDirectoryUrl(shareDirectoryUrl)
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

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUrl,
      const ShareClientOptions& options)
      : m_shareDirectoryUrl(shareDirectoryUrl)
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

  ShareDirectoryClient ShareDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(_internal::UrlEncodePath(subdirectoryName));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(_internal::UrlEncodePath(fileName));
    return ShareFileClient(builder, m_pipeline);
  }

  ShareDirectoryClient ShareDirectoryClient::WithShareSnapshot(
      const std::string& shareSnapshot) const
  {
    ShareDirectoryClient newClient(*this);
    if (shareSnapshot.empty())
    {
      newClient.m_shareDirectoryUrl.RemoveQueryParameter(_detail::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareDirectoryUrl.AppendQueryParameter(
          _detail::ShareSnapshotQueryParameter, _internal::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Response<Models::CreateDirectoryResult> ShareDirectoryClient::Create(
      const CreateDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::Directory.ToString();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
    }
    if (options.DirectoryPermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.DirectoryPermission.Value();
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }
    auto result = _detail::ShareRestClient::Directory::Create(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateDirectoryResult ret;
    ret.Created = true;
    ret.ETag = std::move(result.Value.ETag);
    ret.IsServerEncrypted = result.Value.IsServerEncrypted;
    ret.LastModified = std::move(result.Value.LastModified);
    ret.SmbProperties = std::move(result.Value.SmbProperties);

    return Azure::Response<Models::CreateDirectoryResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::CreateDirectoryResult> ShareDirectoryClient::CreateIfNotExists(
      const CreateDirectoryOptions& options,
      const Azure::Core::Context& context) const

  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ResourceAlreadyExists)
      {
        Models::CreateDirectoryResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreateDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::DeleteOptions();
    auto result = _detail::ShareRestClient::Directory::Delete(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteDirectoryResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteDirectoryResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteDirectoryResult> ShareDirectoryClient::DeleteIfExists(
      const DeleteDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ShareNotFound || e.ErrorCode == _detail::ParentNotFound
          || e.ErrorCode == _detail::ResourceNotFound)
      {
        Models::DeleteDirectoryResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DirectoryProperties> ShareDirectoryClient::GetProperties(
      const GetDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::GetPropertiesOptions();
    return _detail::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetDirectoryPropertiesResult> ShareDirectoryClient::SetProperties(
      Models::FileSmbProperties smbProperties,
      const SetDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = smbProperties.Attributes.ToString();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = FilePreserveSmbProperties;
    }
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = FilePreserveSmbProperties;
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = smbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = FilePreserveSmbProperties;
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission.Value();
    }
    else if (smbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = smbProperties.PermissionKey;
    }
    else
    {
      protocolLayerOptions.FilePermission = FilePreserveSmbProperties;
    }
    return _detail::ShareRestClient::Directory::SetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDirectoryMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    return _detail::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  ListFilesAndDirectoriesPagedResponse ShareDirectoryClient::ListFilesAndDirectories(
      const ListFilesAndDirectoriesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions
        = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto response = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);

    ListFilesAndDirectoriesPagedResponse pagedResponse;

    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.ShareName = std::move(response.Value.ShareName);
    pagedResponse.ShareSnapshot = std::move(response.Value.ShareSnapshot);
    pagedResponse.DirectoryPath = std::move(response.Value.DirectoryPath);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Directories = std::move(response.Value.SinglePage.DirectoryItems);
    pagedResponse.Files = std::move(response.Value.SinglePage.FileItems);
    pagedResponse.m_shareDirectoryClient = std::make_shared<ShareDirectoryClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  ListDirectoryHandlesPagedResponse ShareDirectoryClient::ListHandles(
      const ListDirectoryHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ListHandlesOptions();
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Recursive = options.Recursive;
    auto response = _detail::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);

    ListDirectoryHandlesPagedResponse pagedResponse;

    pagedResponse.DirectoryHandles = std::move(response.Value.HandleList);
    pagedResponse.m_shareDirectoryClient = std::make_shared<ShareDirectoryClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::ForceCloseDirectoryHandleResult> ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseDirectoryHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = _detail::ShareRestClient::File::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ForceCloseDirectoryHandleResult ret;
    return Azure::Response<Models::ForceCloseDirectoryHandleResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  ForceCloseAllDirectoryHandlesPagedResponse ShareDirectoryClient::ForceCloseAllHandles(
      const ForceCloseAllDirectoryHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.Recursive = options.Recursive;
    auto response = _detail::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);

    ForceCloseAllDirectoryHandlesPagedResponse pagedResponse;

    pagedResponse.NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    pagedResponse.NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    pagedResponse.m_shareDirectoryClient = std::make_shared<ShareDirectoryClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

}}}} // namespace Azure::Storage::Files::Shares

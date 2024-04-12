// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_directory_client.hpp"

#include "azure/storage/files/shares/share_file_client.hpp"
#include "private/package_version.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

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
      : m_shareDirectoryUrl(shareDirectoryUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
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
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const ShareClientOptions& options)
      : m_shareDirectoryUrl(shareDirectoryUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
  {
    ShareClientOptions newOptions = options;

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(
          options.Audience.HasValue()
              ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
              : _internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
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
      : m_shareDirectoryUrl(shareDirectoryUrl), m_allowTrailingDot(options.AllowTrailingDot),
        m_allowSourceTrailingDot(options.AllowSourceTrailingDot),
        m_shareTokenIntent(options.ShareTokenIntent)
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
    ShareDirectoryClient subdirectoryClient(builder, m_pipeline);
    subdirectoryClient.m_allowTrailingDot = m_allowTrailingDot;
    subdirectoryClient.m_allowSourceTrailingDot = m_allowSourceTrailingDot;
    subdirectoryClient.m_shareTokenIntent = m_shareTokenIntent;
    return subdirectoryClient;
  }

  ShareFileClient ShareDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(_internal::UrlEncodePath(fileName));
    ShareFileClient fileClient(builder, m_pipeline);
    fileClient.m_allowTrailingDot = m_allowTrailingDot;
    fileClient.m_allowSourceTrailingDot = m_allowSourceTrailingDot;
    fileClient.m_shareTokenIntent = m_shareTokenIntent;
    return fileClient;
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
    auto protocolLayerOptions = _detail::DirectoryClient::CreateDirectoryOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    if (options.SmbProperties.Attributes.GetValues().empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::Directory.ToString();
    }
    else
    {
      protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
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
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
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
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto result = _detail::DirectoryClient::Create(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
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
  Azure::Response<ShareFileClient> ShareDirectoryClient::RenameFile(
      const std::string& fileName,
      const std::string& destinationFilePath,
      const RenameFileOptions& options,
      const Azure::Core::Context& context) const
  {
    auto sourceFileUrl = m_shareDirectoryUrl;
    sourceFileUrl.AppendPath(_internal::UrlEncodePath(fileName));

    const std::string& currentPath = m_shareDirectoryUrl.GetPath();
    std::string destinationFileShare = currentPath.substr(0, currentPath.find('/'));
    auto destinationFileUrl = m_shareDirectoryUrl;
    destinationFileUrl.SetPath(_internal::UrlEncodePath(destinationFileShare));
    destinationFileUrl.AppendPath(_internal::UrlEncodePath(destinationFilePath));

    auto protocolLayerOptions = _detail::FileClient::RenameFileOptions();
    protocolLayerOptions.RenameSource = sourceFileUrl.GetAbsoluteUrl();
    protocolLayerOptions.ReplaceIfExists = options.ReplaceIfExists;
    protocolLayerOptions.IgnoreReadOnly = options.IgnoreReadOnly;
    protocolLayerOptions.DestinationLeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission.Value();
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.AllowSourceTrailingDot = m_allowSourceTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    protocolLayerOptions.FileContentType = options.ContentType;

    auto response = _detail::FileClient::Rename(
        *m_pipeline, destinationFileUrl, protocolLayerOptions, context);

    auto renamedFileClient = ShareFileClient(destinationFileUrl, m_pipeline);
    renamedFileClient.m_allowTrailingDot = m_allowTrailingDot;
    renamedFileClient.m_allowSourceTrailingDot = m_allowSourceTrailingDot;
    renamedFileClient.m_shareTokenIntent = m_shareTokenIntent;
    return Azure::Response<ShareFileClient>(
        std::move(renamedFileClient), std::move(response.RawResponse));
  }

  Azure::Response<ShareDirectoryClient> ShareDirectoryClient::RenameSubdirectory(
      const std::string& subdirectoryName,
      const std::string& destinationDirectoryPath,
      const RenameDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    auto sourceDirectoryUrl = m_shareDirectoryUrl;
    sourceDirectoryUrl.AppendPath(_internal::UrlEncodePath(subdirectoryName));

    const std::string& currentPath = m_shareDirectoryUrl.GetPath();
    std::string destinationFileShare = currentPath.substr(0, currentPath.find('/'));
    auto destinationDirectoryUrl = m_shareDirectoryUrl;
    destinationDirectoryUrl.SetPath(_internal::UrlEncodePath(destinationFileShare));
    destinationDirectoryUrl.AppendPath(_internal::UrlEncodePath(destinationDirectoryPath));

    auto protocolLayerOptions = _detail::DirectoryClient::RenameDirectoryOptions();
    protocolLayerOptions.RenameSource = sourceDirectoryUrl.GetAbsoluteUrl();
    protocolLayerOptions.ReplaceIfExists = options.ReplaceIfExists;
    protocolLayerOptions.IgnoreReadOnly = options.IgnoreReadOnly;
    protocolLayerOptions.DestinationLeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.ToString();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = options.SmbProperties.LastWrittenOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.SmbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = options.SmbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    if (options.FilePermission.HasValue())
    {
      protocolLayerOptions.FilePermission = options.FilePermission.Value();
    }
    else if (options.SmbProperties.PermissionKey.HasValue())
    {
      protocolLayerOptions.FilePermissionKey = options.SmbProperties.PermissionKey;
    }
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.AllowSourceTrailingDot = m_allowSourceTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;

    auto response = _detail::DirectoryClient::Rename(
        *m_pipeline, destinationDirectoryUrl, protocolLayerOptions, context);

    auto renamedSubdirectoryClient = ShareDirectoryClient(destinationDirectoryUrl, m_pipeline);
    renamedSubdirectoryClient.m_allowTrailingDot = m_allowTrailingDot;
    renamedSubdirectoryClient.m_allowSourceTrailingDot = m_allowSourceTrailingDot;
    renamedSubdirectoryClient.m_shareTokenIntent = m_shareTokenIntent;
    return Azure::Response<ShareDirectoryClient>(
        std::move(renamedSubdirectoryClient), std::move(response.RawResponse));
  }

  Azure::Response<Models::DeleteDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::DirectoryClient::DeleteDirectoryOptions();
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto result = _detail::DirectoryClient::Delete(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
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
    auto protocolLayerOptions = _detail::DirectoryClient::GetDirectoryPropertiesOptions();
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    return _detail::DirectoryClient::GetProperties(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetDirectoryPropertiesResult> ShareDirectoryClient::SetProperties(
      Models::FileSmbProperties smbProperties,
      const SetDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::DirectoryClient::SetDirectoryPropertiesOptions();
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
    if (smbProperties.ChangedOn.HasValue())
    {
      protocolLayerOptions.FileChangeTime = smbProperties.ChangedOn.Value().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
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
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    return _detail::DirectoryClient::SetProperties(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SetDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDirectoryMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::DirectoryClient::SetDirectoryMetadataOptions();
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    return _detail::DirectoryClient::SetMetadata(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
  }

  ListFilesAndDirectoriesPagedResponse ShareDirectoryClient::ListFilesAndDirectories(
      const ListFilesAndDirectoriesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions
        = _detail::DirectoryClient::ListDirectoryFilesAndDirectoriesSegmentOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    protocolLayerOptions.IncludeExtendedInfo = options.IncludeExtendedInfo;
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto response = _detail::DirectoryClient::ListFilesAndDirectoriesSegment(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);

    ListFilesAndDirectoriesPagedResponse pagedResponse;

    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.ShareName = std::move(response.Value.ShareName);
    pagedResponse.ShareSnapshot = response.Value.ShareSnapshot.ValueOr(std::string());
    if (response.Value.Encoded)
    {
      pagedResponse.DirectoryPath = Core::Url::Decode(response.Value.DirectoryPath);
    }
    else
    {
      pagedResponse.DirectoryPath = std::move(response.Value.DirectoryPath);
    }
    if (response.Value.Prefix.Encoded)
    {
      pagedResponse.Prefix = Core::Url::Decode(response.Value.Prefix.Content);
    }
    else
    {
      pagedResponse.Prefix = std::move(response.Value.Prefix.Content);
    }
    for (auto& item : response.Value.Segment.DirectoryItems)
    {
      Models::DirectoryItem directoryItem;
      if (item.Name.Encoded)
      {
        directoryItem.Name = Core::Url::Decode(item.Name.Content);
      }
      else
      {
        directoryItem.Name = std::move(item.Name.Content);
      }
      directoryItem.Details = std::move(item.Details);
      pagedResponse.Directories.push_back(std::move(directoryItem));
    }
    for (auto& item : response.Value.Segment.FileItems)
    {
      Models::FileItem fileItem;
      if (item.Name.Encoded)
      {
        fileItem.Name = Core::Url::Decode(item.Name.Content);
      }
      else
      {
        fileItem.Name = std::move(item.Name.Content);
      }
      fileItem.Details = std::move(item.Details);
      pagedResponse.Files.push_back(std::move(fileItem));
    }
    pagedResponse.DirectoryId = response.Value.DirectoryId.ValueOr(std::string());
    pagedResponse.m_shareDirectoryClient = std::make_shared<ShareDirectoryClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.NextMarker;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  ListDirectoryHandlesPagedResponse ShareDirectoryClient::ListHandles(
      const ListDirectoryHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::DirectoryClient::ListDirectoryHandlesOptions();
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Recursive = options.Recursive;
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto response = _detail::DirectoryClient::ListHandles(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);

    ListDirectoryHandlesPagedResponse pagedResponse;

    for (auto& handle : response.Value.HandleList)
    {
      Models::HandleItem directoryHandle;
      if (handle.Path.Encoded)
      {
        directoryHandle.Path = Core::Url::Decode(handle.Path.Content);
      }
      else
      {
        directoryHandle.Path = std::move(handle.Path.Content);
      }
      directoryHandle.ClientIp = std::move(handle.ClientIp);
      directoryHandle.ClientName = std::move(handle.ClientName);
      directoryHandle.FileId = std::move(handle.FileId);
      directoryHandle.HandleId = std::move(handle.HandleId);
      directoryHandle.LastReconnectedOn = std::move(handle.LastReconnectedOn);
      directoryHandle.OpenedOn = std::move(handle.OpenedOn);
      directoryHandle.ParentId = std::move(handle.ParentId);
      directoryHandle.SessionId = std::move(handle.SessionId);
      if (!handle.AccessRightList.empty())
      {
        Models::ShareFileHandleAccessRights accessRights;
        for (auto& accessRight : handle.AccessRightList)
        {
          accessRights |= Models::ShareFileHandleAccessRights(accessRight.ToString());
        }
        directoryHandle.AccessRights = std::move(accessRights);
      }

      pagedResponse.DirectoryHandles.push_back(std::move(directoryHandle));
    }
    pagedResponse.m_shareDirectoryClient = std::make_shared<ShareDirectoryClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    if (!response.Value.NextMarker.empty())
    {
      pagedResponse.NextPageToken = response.Value.NextMarker;
    }
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::ForceCloseDirectoryHandleResult> ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseDirectoryHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::DirectoryClient::ForceDirectoryCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto result = _detail::DirectoryClient::ForceCloseHandles(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);
    Models::ForceCloseDirectoryHandleResult ret;
    return Azure::Response<Models::ForceCloseDirectoryHandleResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  ForceCloseAllDirectoryHandlesPagedResponse ShareDirectoryClient::ForceCloseAllHandles(
      const ForceCloseAllDirectoryHandlesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::DirectoryClient::ForceDirectoryCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    protocolLayerOptions.AllowTrailingDot = m_allowTrailingDot;
    protocolLayerOptions.FileRequestIntent = m_shareTokenIntent;
    auto response = _detail::DirectoryClient::ForceCloseHandles(
        *m_pipeline, m_shareDirectoryUrl, protocolLayerOptions, context);

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

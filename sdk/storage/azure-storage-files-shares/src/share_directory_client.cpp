// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_directory_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>

#include "azure/storage/files/shares/share_file_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareDirectoryClient ShareDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& directoryName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::_detail::ParseConnectionString(connectionString);
    auto directoryUrl = std::move(parsedConnectionString.FileServiceUrl);
    directoryUrl.AppendPath(Storage::_detail::UrlEncodePath(shareName));
    directoryUrl.AppendPath(Storage::_detail::UrlEncodePath(directoryName));

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
        std::make_unique<Storage::_detail::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = newOptions.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        Storage::_detail::FileServicePackageName,
        PackageVersion::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUrl,
      const ShareClientOptions& options)
      : m_shareDirectoryUrl(shareDirectoryUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        Storage::_detail::FileServicePackageName,
        PackageVersion::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  ShareDirectoryClient ShareDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(Storage::_detail::UrlEncodePath(subdirectoryName));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(Storage::_detail::UrlEncodePath(fileName));
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
          _detail::ShareSnapshotQueryParameter,
          Storage::_detail::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::Create(
      const CreateShareDirectoryOptions& options,
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
      protocolLayerOptions.FileCreationTime = options.SmbProperties.CreatedOn.GetValue().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().ToString(
              Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
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
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }
    auto result = _detail::ShareRestClient::Directory::Create(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateShareDirectoryResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.IsServerEncrypted = result->IsServerEncrypted;
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    ret.SmbProperties = std::move(result->SmbProperties);

    return Azure::Response<Models::CreateShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::CreateIfNotExists(
      const CreateShareDirectoryOptions& options,
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
        Models::CreateShareDirectoryResult ret;
        ret.Created = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Response<Models::CreateShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteShareDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::DeleteOptions();
    auto result = _detail::ShareRestClient::Directory::Delete(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteShareDirectoryResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::DeleteIfExists(
      const DeleteShareDirectoryOptions& options,
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
        Models::DeleteShareDirectoryResult ret;
        ret.Deleted = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Response<Models::DeleteShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::GetShareDirectoryPropertiesResult> ShareDirectoryClient::GetProperties(
      const GetShareDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::GetPropertiesOptions();
    return _detail::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetShareDirectoryPropertiesResult> ShareDirectoryClient::SetProperties(
      Models::FileSmbProperties smbProperties,
      const SetShareDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = smbProperties.Attributes.ToString();
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.GetValue().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime = smbProperties.LastWrittenOn.GetValue().ToString(
          Azure::DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileLastWriteTime = std::string(FileDefaultTimeValue);
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
      protocolLayerOptions.FilePermission = std::string(FileInheritPermission);
    }
    return _detail::ShareRestClient::Directory::SetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::SetShareDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareDirectoryMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    return _detail::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Response<Models::ListFilesAndDirectoriesSinglePageResult>
  ShareDirectoryClient::ListFilesAndDirectoriesSinglePage(
      const ListFilesAndDirectoriesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions
        = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
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

    return Azure::Response<Models::ListFilesAndDirectoriesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Response<Models::ListShareDirectoryHandlesSinglePageResult>
  ShareDirectoryClient::ListHandlesSinglePage(
      const ListShareDirectoryHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = _detail::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ListShareDirectoryHandlesSinglePageResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Handles = std::move(result->HandleList);

    return Azure::Response<Models::ListShareDirectoryHandlesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Response<Models::ForceCloseShareDirectoryHandleResult>
  ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseShareDirectoryHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = _detail::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = _detail::ShareRestClient::File::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ForceCloseShareDirectoryHandleResult ret;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Response<Models::ForceCloseShareDirectoryHandleResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Response<Models::ForceCloseAllShareDirectoryHandlesSinglePageResult>
  ShareDirectoryClient::ForceCloseAllHandlesSinglePage(
      const ForceCloseAllShareDirectoryHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    return _detail::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

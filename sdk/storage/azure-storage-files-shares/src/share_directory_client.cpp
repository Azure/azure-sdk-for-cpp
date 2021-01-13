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
#include <azure/storage/common/storage_retry_policy.hpp>

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

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUri,
      const ShareClientOptions& options)
      : m_shareDirectoryUri(shareDirectoryUri)
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

  ShareDirectoryClient ShareDirectoryClient::GetSubdirectoryClient(
      const std::string& subDirectoryName) const
  {
    auto builder = m_shareDirectoryUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(subDirectoryName));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareDirectoryClient::GetFileClient(const std::string& filePath) const
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

  Azure::Core::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::Create(
      const CreateShareDirectoryOptions& options) const
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
    auto result = Details::ShareRestClient::Directory::Create(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::CreateShareDirectoryResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.FileAttributes = result->FileAttributes;
    ret.FileCreatedOn = std::move(result->FileCreatedOn);
    ret.FileLastWrittenOn = std::move(result->FileLastWrittenOn);
    ret.FilePermissionKey = std::move(result->FilePermissionKey);
    ret.FileChangedOn = std::move(result->FileChangedOn);
    ret.FileId = std::move(result->FileId);
    ret.FileParentId = std::move(result->FileParentId);
    ret.IsServerEncrypted = result->IsServerEncrypted;
    ret.LastModified = std::move(result->LastModified);

    return Azure::Core::Response<Models::CreateShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::CreateIfNotExists(
      const CreateShareDirectoryOptions& options) const

  {
    try
    {
      return Create(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ResourceAlreadyExists)
      {
        Models::CreateShareDirectoryResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteShareDirectoryOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::DeleteOptions();
    auto result = Details::ShareRestClient::Directory::Delete(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::DeleteShareDirectoryResult ret;
    ret.Deleted = true;
    return Azure::Core::Response<Models::DeleteShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::DeleteIfExists(
      const DeleteShareDirectoryOptions& options) const
  {
    try
    {
      return Delete(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareNotFound || e.ErrorCode == Details::ParentNotFound
          || e.ErrorCode == Details::ResourceNotFound)
      {
        Models::DeleteShareDirectoryResult ret;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetShareDirectoryPropertiesResult>
  ShareDirectoryClient::GetProperties(const GetShareDirectoryPropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::GetPropertiesOptions();
    return Details::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareDirectoryPropertiesResult>
  ShareDirectoryClient::SetProperties(
      Models::FileShareSmbProperties smbProperties,
      const SetShareDirectoryPropertiesOptions& options) const
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

  Azure::Core::Response<Models::SetShareDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareDirectoryMetadataOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    return Details::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>
  ShareDirectoryClient::ListFilesAndDirectoriesSinglePage(
      const ListFilesAndDirectoriesSinglePageOptions& options) const
  {
    auto protocolLayerOptions
        = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
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

  Azure::Core::Response<Models::ListShareDirectoryHandlesSinglePageResult>
  ShareDirectoryClient::ListHandlesSinglePage(
      const ListShareDirectoryHandlesSinglePageOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = Details::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::ListShareDirectoryHandlesSinglePageResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Handles = std::move(result->HandleList);

    return Azure::Core::Response<Models::ListShareDirectoryHandlesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseShareDirectoryHandleResult>
  ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseShareDirectoryHandleOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = Details::ShareRestClient::File::ForceCloseHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
    return Azure::Core::Response<Models::ForceCloseShareDirectoryHandleResult>(
        Models::ForceCloseShareDirectoryHandleResult(), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseAllShareDirectoryHandlesResult>
  ShareDirectoryClient::ForceCloseAllHandles(
      const ForceCloseAllShareDirectoryHandlesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = c_FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    return Details::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

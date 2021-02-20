// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_directory_client.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/shares/share_file_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  ShareDirectoryClient ShareDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& shareName,
      const std::string& directoryName,
      const ShareClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUrl = std::move(parsedConnectionString.FileServiceUrl);
    directoryUrl.AppendPath(Storage::Details::UrlEncodePath(shareName));
    directoryUrl.AppendPath(Storage::Details::UrlEncodePath(directoryName));

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

  ShareDirectoryClient::ShareDirectoryClient(
      const std::string& shareDirectoryUrl,
      const ShareClientOptions& options)
      : m_shareDirectoryUrl(shareDirectoryUrl)
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

  ShareDirectoryClient ShareDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(subdirectoryName));
    return ShareDirectoryClient(builder, m_pipeline);
  }

  ShareFileClient ShareDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_shareDirectoryUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileName));
    return ShareFileClient(builder, m_pipeline);
  }

  ShareDirectoryClient ShareDirectoryClient::WithShareSnapshot(
      const std::string& shareSnapshot) const
  {
    ShareDirectoryClient newClient(*this);
    if (shareSnapshot.empty())
    {
      newClient.m_shareDirectoryUrl.RemoveQueryParameter(Details::ShareSnapshotQueryParameter);
    }
    else
    {
      newClient.m_shareDirectoryUrl.AppendQueryParameter(
          Details::ShareSnapshotQueryParameter,
          Storage::Details::UrlEncodeQueryParameter(shareSnapshot));
    }
    return newClient;
  }

  Azure::Core::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::Create(
      const CreateShareDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::CreateOptions();
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.FileAttributes = options.SmbProperties.Attributes.Get();
    if (protocolLayerOptions.FileAttributes.empty())
    {
      protocolLayerOptions.FileAttributes = Models::FileAttributes::Directory.Get();
    }
    if (options.SmbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime
          = options.SmbProperties.CreatedOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (options.SmbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = options.SmbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
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
    auto result = Details::ShareRestClient::Directory::Create(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::CreateShareDirectoryResult ret;
    ret.Created = true;
    ret.ETag = std::move(result->ETag);
    ret.IsServerEncrypted = result->IsServerEncrypted;
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    ret.SmbProperties = std::move(result->SmbProperties);

    return Azure::Core::Response<Models::CreateShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateShareDirectoryResult> ShareDirectoryClient::CreateIfNotExists(
      const CreateShareDirectoryOptions& options,
      const Azure::Core::Context& context) const

  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ResourceAlreadyExists)
      {
        Models::CreateShareDirectoryResult ret;
        ret.Created = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Core::Response<Models::CreateShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::Delete(
      const DeleteShareDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Directory::DeleteOptions();
    auto result = Details::ShareRestClient::Directory::Delete(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::DeleteShareDirectoryResult ret;
    ret.Deleted = true;
    return Azure::Core::Response<Models::DeleteShareDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteShareDirectoryResult> ShareDirectoryClient::DeleteIfExists(
      const DeleteShareDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ShareNotFound || e.ErrorCode == Details::ParentNotFound
          || e.ErrorCode == Details::ResourceNotFound)
      {
        Models::DeleteShareDirectoryResult ret;
        ret.Deleted = false;
        ret.RequestId = std::move(e.RequestId);
        return Azure::Core::Response<Models::DeleteShareDirectoryResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetShareDirectoryPropertiesResult>
  ShareDirectoryClient::GetProperties(
      const GetShareDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Directory::GetPropertiesOptions();
    return Details::ShareRestClient::Directory::GetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareDirectoryPropertiesResult>
  ShareDirectoryClient::SetProperties(
      Models::FileSmbProperties smbProperties,
      const SetShareDirectoryPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetPropertiesOptions();
    protocolLayerOptions.FileAttributes = smbProperties.Attributes.Get();
    if (smbProperties.CreatedOn.HasValue())
    {
      protocolLayerOptions.FileCreationTime = smbProperties.CreatedOn.GetValue().GetRfc3339String(
          Core::DateTime::TimeFractionFormat::AllDigits);
    }
    else
    {
      protocolLayerOptions.FileCreationTime = std::string(FileDefaultTimeValue);
    }
    if (smbProperties.LastWrittenOn.HasValue())
    {
      protocolLayerOptions.FileLastWriteTime
          = smbProperties.LastWrittenOn.GetValue().GetRfc3339String(
              Core::DateTime::TimeFractionFormat::AllDigits);
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
    return Details::ShareRestClient::Directory::SetProperties(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetShareDirectoryMetadataResult> ShareDirectoryClient::SetMetadata(
      Storage::Metadata metadata,
      const SetShareDirectoryMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::Directory::SetMetadataOptions();
    protocolLayerOptions.Metadata = std::move(metadata);
    return Details::ShareRestClient::Directory::SetMetadata(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>
  ShareDirectoryClient::ListFilesAndDirectoriesSinglePage(
      const ListFilesAndDirectoriesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions
        = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto result = Details::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
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

    return Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ListShareDirectoryHandlesSinglePageResult>
  ShareDirectoryClient::ListHandlesSinglePage(
      const ListShareDirectoryHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Recursive = options.Recursive;
    auto result = Details::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ListShareDirectoryHandlesSinglePageResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.Handles = std::move(result->HandleList);

    return Azure::Core::Response<Models::ListShareDirectoryHandlesSinglePageResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseShareDirectoryHandleResult>
  ShareDirectoryClient::ForceCloseHandle(
      const std::string& handleId,
      const ForceCloseShareDirectoryHandleOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    auto protocolLayerOptions = Details::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = handleId;
    auto result = Details::ShareRestClient::File::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
    Models::ForceCloseShareDirectoryHandleResult ret;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::ForceCloseShareDirectoryHandleResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ForceCloseAllShareDirectoryHandlesSinglePageResult>
  ShareDirectoryClient::ForceCloseAllHandlesSinglePage(
      const ForceCloseAllShareDirectoryHandlesSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Recursive = options.Recursive;
    return Details::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryUrl, *m_pipeline, context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

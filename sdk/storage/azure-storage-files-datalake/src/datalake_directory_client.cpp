// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_directory_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeDirectoryClient DataLakeDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& directoryName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    directoryUrl.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    directoryUrl.AppendPath(Storage::Details::UrlEncodePath(directoryName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeDirectoryClient(
          directoryUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeDirectoryClient(directoryUrl.GetAbsoluteUrl(), options);
    }
  }

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUrl, credential, options)
  {
  }

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUrl, credential, options)
  {
  }

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUrl,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUrl, options)
  {
  }

  DataLakeFileClient DataLakeDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_pathUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(fileName));
    return DataLakeFileClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_pathUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(subdirectoryName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(subdirectoryName));
    return DataLakeDirectoryClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  Azure::Core::Response<DataLakeFileClient> DataLakeDirectoryClient::RenameFile(
      const std::string& fileName,
      const std::string& destinationFilePath,
      const RenameDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_pathUrl.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUrl = m_pathUrl;
    destinationDfsUrl.SetPath(
        destinationFileSystem.GetValue() + '/'
        + Storage::Details::UrlEncodePath(destinationFilePath));

    Details::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Mode = Models::PathRenameMode::Legacy;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RenameSource
        = "/" + m_pathUrl.GetPath() + Storage::Details::UrlEncodePath(fileName);
    auto result = Details::DataLakeRestClient::Path::Create(
        destinationDfsUrl, *m_pipeline, context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    // Initialize the file client.
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.SetPath(destinationDfsUrl.GetPath());
    auto renamedFileClient
        = DataLakeFileClient(std::move(destinationDfsUrl), std::move(blobClient), m_pipeline);
    return Azure::Core::Response<DataLakeFileClient>(
        std::move(renamedFileClient), result.ExtractRawResponse());
  }

  Azure::Core::Response<DataLakeDirectoryClient> DataLakeDirectoryClient::RenameSubdirectory(
      const std::string& subdirectoryName,
      const std::string& destinationDirectoryPath,
      const RenameDataLakeSubdirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_pathUrl.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUrl = m_pathUrl;
    destinationDfsUrl.SetPath(
        destinationFileSystem.GetValue() + '/'
        + Storage::Details::UrlEncodePath(destinationDirectoryPath));

    Details::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Mode = Models::PathRenameMode::Legacy;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RenameSource
        = "/" + m_pathUrl.GetPath() + Storage::Details::UrlEncodePath(subdirectoryName);
    auto result = Details::DataLakeRestClient::Path::Create(
        destinationDfsUrl, *m_pipeline, context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    // Initialize the directory client.
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.SetPath(destinationDfsUrl.GetPath());
    auto renamedDirectoryClient
        = DataLakeDirectoryClient(std::move(destinationDfsUrl), std::move(blobClient), m_pipeline);
    return Azure::Core::Response<DataLakeDirectoryClient>(
        std::move(renamedDirectoryClient), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DataLakeDirectoryClient::Delete(
      bool recursive,
      const DeleteDataLakeDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::Delete(deleteOptions, context);
  }

  Azure::Core::Response<Models::DeleteDataLakeDirectoryResult>
  DataLakeDirectoryClient::DeleteIfExists(
      bool recursive,
      const DeleteDataLakeDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::DeleteIfExists(deleteOptions, context);
  }

  Azure::Core::Response<Models::ListPathsSinglePageResult>
  DataLakeDirectoryClient::ListPathsSinglePage(
      bool recursive,
      const ListPathsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Resource = Models::FileSystemResourceType::Filesystem;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.RecursiveRequired = recursive;
    auto currentPath = m_pathUrl.GetPath();
    // Remove the filesystem name and get directory name.
    auto firstSlashPos = currentPath.find_first_of("/");

    if (firstSlashPos == 0 || (firstSlashPos == currentPath.size() + 1U))
    {
      return Details::DataLakeRestClient::FileSystem::ListPaths(
          m_pathUrl, *m_pipeline, context, protocolLayerOptions);
    }
    else
    {
      protocolLayerOptions.Directory
          = currentPath.substr(firstSlashPos + 1U, currentPath.size() - firstSlashPos - 1U);
      auto fileSystemUrl = m_pathUrl;
      fileSystemUrl.SetPath(currentPath.substr(
          0U, currentPath.size() - protocolLayerOptions.Directory.GetValue().size() - 1U));
      return Details::DataLakeRestClient::FileSystem::ListPaths(
          fileSystemUrl, *m_pipeline, context, protocolLayerOptions);
    }
  }

}}}} // namespace Azure::Storage::Files::DataLake

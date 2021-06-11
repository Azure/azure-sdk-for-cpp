// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_directory_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "private/datalake_utilities.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeDirectoryClient DataLakeDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& directoryName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto directoryUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    directoryUrl.AppendPath(_internal::UrlEncodePath(fileSystemName));
    directoryUrl.AppendPath(_internal::UrlEncodePath(directoryName));

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
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
    builder.AppendPath(_internal::UrlEncodePath(fileName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(_internal::UrlEncodePath(fileName));
    return DataLakeFileClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_pathUrl;
    builder.AppendPath(_internal::UrlEncodePath(subdirectoryName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(_internal::UrlEncodePath(subdirectoryName));
    return DataLakeDirectoryClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  Azure::Response<DataLakeFileClient> DataLakeDirectoryClient::RenameFile(
      const std::string& fileName,
      const std::string& destinationFilePath,
      const RenameFileOptions& options,
      const Azure::Core::Context& context) const
  {
    std::string destinationFileSystem;
    if (options.DestinationFileSystem.HasValue())
    {
      destinationFileSystem = options.DestinationFileSystem.Value();
    }
    else
    {
      const std::string& currentPath = m_pathUrl.GetPath();
      destinationFileSystem = currentPath.substr(0, currentPath.find('/'));
    }

    auto sourceDfsUrl = m_pathUrl;
    sourceDfsUrl.AppendPath(_internal::UrlEncodePath(fileName));

    auto destinationDfsUrl = m_pathUrl;
    destinationDfsUrl.SetPath(_internal::UrlEncodePath(destinationFileSystem));
    destinationDfsUrl.AppendPath(_internal::UrlEncodePath(destinationFilePath));

    _detail::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Mode = _detail::PathRenameMode::Legacy;
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
    protocolLayerOptions.RenameSource = "/" + sourceDfsUrl.GetPath();
    auto result = _detail::DataLakeRestClient::Path::Create(
        destinationDfsUrl, *m_pipeline, context, protocolLayerOptions);

    auto renamedBlobClient
        = Blobs::BlobClient(_detail::GetBlobUrlFromUrl(destinationDfsUrl), m_pipeline);
    auto renamedFileClient = DataLakeFileClient(
        std::move(destinationDfsUrl), std::move(renamedBlobClient), m_pipeline);
    return Azure::Response<DataLakeFileClient>(
        std::move(renamedFileClient), std::move(result.RawResponse));
  }

  Azure::Response<DataLakeDirectoryClient> DataLakeDirectoryClient::RenameSubdirectory(
      const std::string& subdirectoryName,
      const std::string& destinationDirectoryPath,
      const RenameSubdirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    std::string destinationFileSystem;
    if (options.DestinationFileSystem.HasValue())
    {
      destinationFileSystem = options.DestinationFileSystem.Value();
    }
    else
    {
      const std::string& currentPath = m_pathUrl.GetPath();
      destinationFileSystem = currentPath.substr(0, currentPath.find('/'));
    }

    auto sourceDfsUrl = m_pathUrl;
    sourceDfsUrl.AppendPath(_internal::UrlEncodePath(subdirectoryName));

    auto destinationDfsUrl = m_pathUrl;
    destinationDfsUrl.SetPath(_internal::UrlEncodePath(destinationFileSystem));
    destinationDfsUrl.AppendPath(_internal::UrlEncodePath(destinationDirectoryPath));

    _detail::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Mode = _detail::PathRenameMode::Legacy;
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
    protocolLayerOptions.RenameSource = "/" + sourceDfsUrl.GetPath();
    auto result = _detail::DataLakeRestClient::Path::Create(
        destinationDfsUrl, *m_pipeline, context, protocolLayerOptions);

    auto renamedBlobClient
        = Blobs::BlobClient(_detail::GetBlobUrlFromUrl(destinationDfsUrl), m_pipeline);
    auto renamedDirectoryClient = DataLakeDirectoryClient(
        std::move(destinationDfsUrl), std::move(renamedBlobClient), m_pipeline);
    return Azure::Response<DataLakeDirectoryClient>(
        std::move(renamedDirectoryClient), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteDirectoryResult> DataLakeDirectoryClient::Delete(
      bool recursive,
      const DeleteDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::Delete(deleteOptions, context);
  }

  Azure::Response<Models::DeleteDirectoryResult> DataLakeDirectoryClient::DeleteIfExists(
      bool recursive,
      const DeleteDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    DeletePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::DeleteIfExists(deleteOptions, context);
  }

  ListPathsPagedResponse DataLakeDirectoryClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Resource = _detail::FileSystemResource::Filesystem;
    protocolLayerOptions.Upn = options.UserPrincipalName;

    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.RecursiveRequired = recursive;

    const std::string currentPath = m_pathUrl.GetPath();
    auto firstSlashPos = std::find(currentPath.begin(), currentPath.end(), '/');
    const std::string fileSystemName(currentPath.begin(), firstSlashPos);
    if (firstSlashPos != currentPath.end())
    {
      ++firstSlashPos;
    }
    const std::string directoryPath(firstSlashPos, currentPath.end());
    if (!directoryPath.empty())
    {
      protocolLayerOptions.Directory = directoryPath;
    }

    auto fileSystemUrl = m_pathUrl;
    fileSystemUrl.SetPath(fileSystemName);

    auto clientCopy = *this;
    std::function<ListPathsPagedResponse(std::string, const Azure::Core::Context&)> func;
    func = [func, clientCopy, protocolLayerOptions, fileSystemUrl](
               std::string continuationToken, const Azure::Core::Context& context) {
      auto protocolLayerOptionsCopy = protocolLayerOptions;
      if (!continuationToken.empty())
      {
        protocolLayerOptionsCopy.ContinuationToken = continuationToken;
      }
      auto response = _detail::DataLakeRestClient::FileSystem::ListPaths(
          fileSystemUrl,
          *clientCopy.m_pipeline,
          _internal::WithReplicaStatus(context),
          protocolLayerOptionsCopy);

      ListPathsPagedResponse pagedResponse;

      pagedResponse.Paths = std::move(response.Value.Items);
      pagedResponse.m_onNextPageFunc = func;
      pagedResponse.CurrentPageToken = continuationToken;
      pagedResponse.NextPageToken = response.Value.ContinuationToken;
      pagedResponse.RawResponse = std::move(response.RawResponse);

      return pagedResponse;
    };

    return func(options.ContinuationToken.ValueOr(std::string()), context);
  }

}}}} // namespace Azure::Storage::Files::DataLake

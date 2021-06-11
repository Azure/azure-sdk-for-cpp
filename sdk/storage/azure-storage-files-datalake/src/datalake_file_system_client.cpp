// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/blobs/protocol/blob_rest_client.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_directory_client.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "private/datalake_constants.hpp"
#include "private/datalake_utilities.hpp"
#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeFileSystemClient DataLakeFileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString
        = Azure::Storage::_internal::ParseConnectionString(connectionString);
    auto fileSystemUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileSystemUrl.AppendPath(_internal::UrlEncodePath(fileSystemName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeFileSystemClient(
          fileSystemUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeFileSystemClient(fileSystemUrl.GetAbsoluteUrl(), options);
    }
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_fileSystemUrl(fileSystemUrl), m_blobContainerClient(
                                            _detail::GetBlobUrlFromUrl(fileSystemUrl),
                                            credential,
                                            _detail::GetBlobClientOptions(options))
  {
    DataLakeClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_fileSystemUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_fileSystemUrl(fileSystemUrl), m_blobContainerClient(
                                            _detail::GetBlobUrlFromUrl(fileSystemUrl),
                                            credential,
                                            _detail::GetBlobClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_fileSystemUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(_internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUrl,
      const DataLakeClientOptions& options)
      : m_fileSystemUrl(fileSystemUrl), m_blobContainerClient(
                                            _detail::GetBlobUrlFromUrl(fileSystemUrl),
                                            _detail::GetBlobClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_fileSystemUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeFileClient DataLakeFileSystemClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_fileSystemUrl;
    builder.AppendPath(_internal::UrlEncodePath(fileName));
    auto blobClient = m_blobContainerClient.GetBlobClient(fileName);
    return DataLakeFileClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeFileSystemClient::GetDirectoryClient(
      const std::string& directoryName) const
  {
    auto builder = m_fileSystemUrl;
    builder.AppendPath(_internal::UrlEncodePath(directoryName));
    return DataLakeDirectoryClient(
        builder, m_blobContainerClient.GetBlobClient(directoryName), m_pipeline);
  }

  Azure::Response<Models::CreateFileSystemResult> DataLakeFileSystemClient::Create(
      const CreateFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::CreateBlobContainerOptions blobOptions;
    blobOptions.Metadata = options.Metadata;
    if (options.AccessType == Models::PublicAccessType::FileSystem)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::BlobContainer;
    }
    else if (options.AccessType == Models::PublicAccessType::Path)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::Blob;
    }
    else if (options.AccessType == Models::PublicAccessType::None)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::None;
    }
    else
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.ToString());
    }
    auto result = m_blobContainerClient.Create(blobOptions, context);
    Models::CreateFileSystemResult ret;
    ret.ETag = std::move(result.Value.ETag);
    ret.LastModified = std::move(result.Value.LastModified);
    ret.Created = true;
    return Azure::Response<Models::CreateFileSystemResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::CreateFileSystemResult> DataLakeFileSystemClient::CreateIfNotExists(
      const CreateFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ContainerAlreadyExists)
      {
        Models::CreateFileSystemResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreateFileSystemResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteFileSystemResult> DataLakeFileSystemClient::Delete(
      const DeleteFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::DeleteBlobContainerOptions blobOptions;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions, context);
    Models::DeleteFileSystemResult ret;
    ret.Deleted = true;
    return Azure::Response<Models::DeleteFileSystemResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::DeleteFileSystemResult> DataLakeFileSystemClient::DeleteIfExists(
      const DeleteFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == _detail::ContainerNotFound)
      {
        Models::DeleteFileSystemResult ret;
        ret.Deleted = false;
        return Azure::Response<Models::DeleteFileSystemResult>(ret, std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::FileSystemProperties> DataLakeFileSystemClient::GetProperties(
      const GetFileSystemPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobContainerPropertiesOptions blobOptions;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions, context);
    Models::FileSystemProperties ret;
    ret.ETag = std::move(result.Value.ETag);
    ret.LastModified = std::move(result.Value.LastModified);
    ret.Metadata = std::move(result.Value.Metadata);
    return Azure::Response<Models::FileSystemProperties>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<Models::SetFileSystemMetadataResult> DataLakeFileSystemClient::SetMetadata(
      Storage::Metadata metadata,
      const SetFileSystemMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobContainerMetadataOptions blobOptions;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;

    auto result = m_blobContainerClient.SetMetadata(std::move(metadata), blobOptions, context);
    Models::SetFileSystemMetadataResult ret;
    ret.ETag = std::move(result.Value.ETag);
    ret.LastModified = std::move(result.Value.LastModified);
    return Azure::Response<Models::SetFileSystemMetadataResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  ListPathsPagedResponse DataLakeFileSystemClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Resource = _detail::FileSystemResource::Filesystem;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.RecursiveRequired = recursive;

    auto clientCopy = *this;
    std::function<ListPathsPagedResponse(std::string, const Azure::Core::Context&)> func;
    func = [func, clientCopy, protocolLayerOptions](
               std::string continuationToken, const Azure::Core::Context& context) {
      auto protocolLayerOptionsCopy = protocolLayerOptions;
      if (!continuationToken.empty())
      {
        protocolLayerOptionsCopy.ContinuationToken = continuationToken;
      }
      auto response = _detail::DataLakeRestClient::FileSystem::ListPaths(
          clientCopy.m_fileSystemUrl,
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

  Azure::Response<Models::FileSystemAccessPolicy> DataLakeFileSystemClient::GetAccessPolicy(
      const GetFileSystemAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobContainerAccessPolicyOptions blobOptions;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto response = m_blobContainerClient.GetAccessPolicy(blobOptions, context);
    Models::FileSystemAccessPolicy ret;
    if (response.Value.AccessType == Blobs::Models::PublicAccessType::BlobContainer)
    {
      ret.AccessType = Models::PublicAccessType::FileSystem;
    }
    else if (response.Value.AccessType == Blobs::Models::PublicAccessType::Blob)
    {
      ret.AccessType = Models::PublicAccessType::Path;
    }
    else if (response.Value.AccessType == Blobs::Models::PublicAccessType::None)
    {
      ret.AccessType = Models::PublicAccessType::None;
    }
    else
    {
      ret.AccessType = Models::PublicAccessType(response.Value.AccessType.ToString());
    }
    ret.SignedIdentifiers = std::move(response.Value.SignedIdentifiers);
    return Azure::Response<Models::FileSystemAccessPolicy>(
        std::move(ret), std::move(response.RawResponse));
  }

  Azure::Response<Models::SetFileSystemAccessPolicyResult>
  DataLakeFileSystemClient::SetAccessPolicy(
      const SetFileSystemAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobContainerAccessPolicyOptions blobOptions;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    blobOptions.SignedIdentifiers = options.SignedIdentifiers;
    if (options.AccessType == Models::PublicAccessType::FileSystem)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::BlobContainer;
    }
    else if (options.AccessType == Models::PublicAccessType::Path)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::Blob;
    }
    else if (options.AccessType == Models::PublicAccessType::None)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::None;
    }
    else
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.ToString());
    }
    auto result = m_blobContainerClient.SetAccessPolicy(blobOptions, context);
    Models::SetFileSystemAccessPolicyResult ret;

    ret.ETag = std::move(result.Value.ETag);
    ret.LastModified = std::move(result.Value.LastModified);
    return Azure::Response<Models::SetFileSystemAccessPolicyResult>(
        std::move(ret), std::move(result.RawResponse));
  }

  Azure::Response<DataLakeFileClient> DataLakeFileSystemClient::RenameFile(
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
      const std::string& currentPath = m_fileSystemUrl.GetPath();
      destinationFileSystem = currentPath.substr(0, currentPath.find('/'));
    }

    auto sourceDfsUrl = m_fileSystemUrl;
    sourceDfsUrl.AppendPath(_internal::UrlEncodePath(fileName));

    auto destinationDfsUrl = m_fileSystemUrl;
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

  Azure::Response<DataLakeDirectoryClient> DataLakeFileSystemClient::RenameDirectory(
      const std::string& directoryName,
      const std::string& destinationDirectoryPath,
      const RenameDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    std::string destinationFileSystem;
    if (options.DestinationFileSystem.HasValue())
    {
      destinationFileSystem = options.DestinationFileSystem.Value();
    }
    else
    {
      const std::string& currentPath = m_fileSystemUrl.GetPath();
      destinationFileSystem = currentPath.substr(0, currentPath.find('/'));
    }

    auto sourceDfsUrl = m_fileSystemUrl;
    sourceDfsUrl.AppendPath(_internal::UrlEncodePath(directoryName));

    auto destinationDfsUrl = m_fileSystemUrl;
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

}}}} // namespace Azure::Storage::Files::DataLake

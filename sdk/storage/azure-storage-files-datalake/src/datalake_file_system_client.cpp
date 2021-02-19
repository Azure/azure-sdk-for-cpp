// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/blobs/protocol/blob_rest_client.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/files/datalake/datalake_constants.hpp"
#include "azure/storage/files/datalake/datalake_directory_client.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeFileSystemClient DataLakeFileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileSystemUrl = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileSystemUrl.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));

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
                                            Details::GetBlobUrlFromUrl(fileSystemUrl),
                                            credential,
                                            Details::GetBlobClientOptions(options))
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

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_fileSystemUrl(fileSystemUrl), m_blobContainerClient(
                                            Details::GetBlobUrlFromUrl(fileSystemUrl),
                                            credential,
                                            Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    Azure::Core::Http::TokenRequestOptions tokenOptions;
    tokenOptions.Scopes.emplace_back(Storage::Details::StorageScope);
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
                credential, tokenOptions),
            options));
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUrl,
      const DataLakeClientOptions& options)
      : m_fileSystemUrl(fileSystemUrl), m_blobContainerClient(
                                            Details::GetBlobUrlFromUrl(fileSystemUrl),
                                            Details::GetBlobClientOptions(options))
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

  DataLakeFileClient DataLakeFileSystemClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_fileSystemUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileName));
    auto blobClient = m_blobContainerClient.GetBlobClient(fileName);
    return DataLakeFileClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeFileSystemClient::GetDirectoryClient(
      const std::string& directoryName) const
  {
    auto builder = m_fileSystemUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(directoryName));
    return DataLakeDirectoryClient(
        builder, m_blobContainerClient.GetBlobClient(directoryName), m_pipeline);
  }

  Azure::Core::Response<Models::CreateDataLakeFileSystemResult> DataLakeFileSystemClient::Create(
      const CreateDataLakeFileSystemOptions& options,
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
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.Get());
    }
    auto result = m_blobContainerClient.Create(blobOptions, context);
    Models::CreateDataLakeFileSystemResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Created = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::CreateDataLakeFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakeFileSystemResult>
  DataLakeFileSystemClient::CreateIfNotExists(
      const CreateDataLakeFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ContainerAlreadyExists)
      {
        Models::CreateDataLakeFileSystemResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateDataLakeFileSystemResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> DataLakeFileSystemClient::Delete(
      const DeleteDataLakeFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::DeleteBlobContainerOptions blobOptions;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions, context);
    Models::DeleteDataLakeFileSystemResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>
  DataLakeFileSystemClient::DeleteIfExists(
      const DeleteDataLakeFileSystemOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ContainerNotFound)
      {
        Models::DeleteDataLakeFileSystemResult ret;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>(
            ret, std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult>
  DataLakeFileSystemClient::GetProperties(
      const GetDataLakeFileSystemPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobContainerPropertiesOptions blobOptions;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions, context);
    Models::GetDataLakeFileSystemPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Metadata = std::move(result->Metadata);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult>
  DataLakeFileSystemClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDataLakeFileSystemMetadataOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::SetBlobContainerMetadataOptions blobOptions;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    if (options.AccessConditions.IfUnmodifiedSince.HasValue())
    {
      std::abort();
    }
    auto result = m_blobContainerClient.SetMetadata(std::move(metadata), blobOptions, context);
    Models::SetDataLakeFileSystemMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ListPathsSinglePageResult>
  DataLakeFileSystemClient::ListPathsSinglePage(
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
    return Details::DataLakeRestClient::FileSystem::ListPaths(
        m_fileSystemUrl, *m_pipeline, context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetDataLakeFileSystemAccessPolicyResult>
  DataLakeFileSystemClient::GetAccessPolicy(
      const GetDataLakeFileSystemAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::GetBlobContainerAccessPolicyOptions blobOptions;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetAccessPolicy(blobOptions, context);
    Models::GetDataLakeFileSystemAccessPolicyResult ret;
    if (result->AccessType == Blobs::Models::PublicAccessType::BlobContainer)
    {
      ret.AccessType = Models::PublicAccessType::FileSystem;
    }
    else if (result->AccessType == Blobs::Models::PublicAccessType::Blob)
    {
      ret.AccessType = Models::PublicAccessType::Path;
    }
    else if (result->AccessType == Blobs::Models::PublicAccessType::None)
    {
      ret.AccessType = Models::PublicAccessType::None;
    }
    else
    {
      ret.AccessType = Models::PublicAccessType(result->AccessType.Get());
    }
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.SignedIdentifiers = std::move(result->SignedIdentifiers);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::GetDataLakeFileSystemAccessPolicyResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakeFileSystemAccessPolicyResult>
  DataLakeFileSystemClient::SetAccessPolicy(
      const SetDataLakeFileSystemAccessPolicyOptions& options,
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
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.Get());
    }
    auto result = m_blobContainerClient.SetAccessPolicy(blobOptions, context);
    Models::SetDataLakeFileSystemAccessPolicyResult ret;

    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakeFileSystemAccessPolicyResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<DataLakeFileClient> DataLakeFileSystemClient::RenameFile(
      const std::string& fileName,
      const std::string& destinationFilePath,
      const RenameDataLakeFileOptions& options,
      const Azure::Core::Context& context) const
  {
    return this->GetDirectoryClient("").RenameFile(fileName, destinationFilePath, options, context);
  }

  Azure::Core::Response<DataLakeDirectoryClient> DataLakeFileSystemClient::RenameDirectory(
      const std::string& directoryName,
      const std::string& destinationDirectoryPath,
      const RenameDataLakeDirectoryOptions& options,
      const Azure::Core::Context& context) const
  {
    return this->GetDirectoryClient("").RenameSubdirectory(
        directoryName, destinationDirectoryPath, options, context);
  }

}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"

#include "azure/core/credentials/policy/policies.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_version.hpp"
#include "azure/storage/files/datalake/datalake_directory_client.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobContainerClientOptions GetBlobContainerClientOptions(
        const FileSystemClientOptions& options)
    {
      Blobs::BlobContainerClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(p->Clone());
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(p->Clone());
      }
      return blobOptions;
    }
  } // namespace

  FileSystemClient FileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const FileSystemClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileSystemUri = std::move(parsedConnectionString.DataLakeServiceUri);
    fileSystemUri.AppendPath(fileSystemName, true);

    if (parsedConnectionString.KeyCredential)
    {
      return FileSystemClient(
          fileSystemUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return FileSystemClient(fileSystemUri.GetAbsoluteUrl(), options);
    }
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const FileSystemClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
  {

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, DataLakeServiceVersion));
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

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
      const FileSystemClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, DataLakeServiceVersion));
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

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      const FileSystemClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            GetBlobContainerClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, DataLakeServiceVersion));
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

  PathClient FileSystemClient::GetPathClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(path, true);
    return PathClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  FileClient FileSystemClient::GetFileClient(const std::string& path) const
  {

    auto builder = m_dfsUri;
    builder.AppendPath(path, true);
    return FileClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  DirectoryClient FileSystemClient::GetDirectoryClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(path, true);
    return DirectoryClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  Azure::Core::Response<CreateFileSystemResult> FileSystemClient::Create(
      const CreateFileSystemOptions& options) const
  {
    Blobs::CreateContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Metadata = options.Metadata;
    auto result = m_blobContainerClient.Create(blobOptions);
    CreateFileSystemResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<CreateFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileSystemDeleteResult> FileSystemClient::Delete(
      const DeleteFileSystemOptions& options) const
  {
    Blobs::DeleteContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions);
    FileSystemDeleteResult ret;
    return Azure::Core::Response<FileSystemDeleteResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<GetFileSystemPropertiesResult> FileSystemClient::GetProperties(
      const GetFileSystemPropertiesOptions& options) const
  {
    Blobs::GetContainerPropertiesOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions);
    GetFileSystemPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Metadata = std::move(result->Metadata);
    return Azure::Core::Response<GetFileSystemPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<SetFileSystemMetadataResult> FileSystemClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const SetFileSystemMetadataOptions& options) const
  {
    Blobs::SetContainerMetadataOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = m_blobContainerClient.SetMetadata(metadata, blobOptions);
    SetFileSystemMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<SetFileSystemMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<ListPathsResult> FileSystemClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options) const
  {
    DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Directory = options.Directory;
    protocolLayerOptions.RecursiveRequired = recursive;
    return DataLakeRestClient::FileSystem::ListPaths(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

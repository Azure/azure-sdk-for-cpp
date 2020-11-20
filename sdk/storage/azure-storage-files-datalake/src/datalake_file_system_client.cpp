// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/datalake/datalake_directory_client.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobClientOptions GetBlobContainerClientOptions(const DataLakeClientOptions& options)
    {
      Blobs::BlobClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(p->Clone());
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(p->Clone());
      }
      blobOptions.RetryOptions = options.RetryOptions;
      blobOptions.RetryOptions.SecondaryHostForRetryReads
          = Details::GetBlobUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
      return blobOptions;
    }
  } // namespace

  FileSystemClient FileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileSystemUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileSystemUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));

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
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
  {

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUriFromUri(fileSystemUri),
            GetBlobContainerClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }

    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  PathClient FileSystemClient::GetPathClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    return PathClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  FileClient FileSystemClient::GetFileClient(const std::string& path) const
  {

    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    auto blobClient = m_blobContainerClient.GetBlobClient(path);
    auto blockBlobClient = blobClient.AsBlockBlobClient();
    return FileClient(
        std::move(builder), std::move(blobClient), std::move(blockBlobClient), m_pipeline);
  }

  DirectoryClient FileSystemClient::GetDirectoryClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    return DirectoryClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  Azure::Core::Response<Models::CreateFileSystemResult> FileSystemClient::Create(
      const CreateFileSystemOptions& options) const
  {
    Blobs::CreateContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Metadata = options.Metadata;
    auto result = m_blobContainerClient.Create(blobOptions);
    Models::CreateFileSystemResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<Models::CreateFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::FileSystemDeleteResult> FileSystemClient::Delete(
      const DeleteFileSystemOptions& options) const
  {
    Blobs::DeleteContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions);
    Models::FileSystemDeleteResult ret;
    return Azure::Core::Response<Models::FileSystemDeleteResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::GetFileSystemPropertiesResult> FileSystemClient::GetProperties(
      const GetFileSystemPropertiesOptions& options) const
  {
    Blobs::GetContainerPropertiesOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions);
    Models::GetFileSystemPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Metadata = std::move(result->Metadata);
    return Azure::Core::Response<Models::GetFileSystemPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetFileSystemMetadataResult> FileSystemClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const SetFileSystemMetadataOptions& options) const
  {
    Blobs::SetContainerMetadataOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = m_blobContainerClient.SetMetadata(metadata, blobOptions);
    Models::SetFileSystemMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<Models::SetFileSystemMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ListPathsResult> FileSystemClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options) const
  {
    Details::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Directory = options.Directory;
    protocolLayerOptions.RecursiveRequired = recursive;
    return Details::DataLakeRestClient::FileSystem::ListPaths(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake_file_system_client.hpp"

#include "blobs/protocol/blob_rest_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/constants.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/storage_version.hpp"
#include "credentials/policy/policies.hpp"
#include "datalake/datalake_directory_client.hpp"
#include "datalake/datalake_file_client.hpp"
#include "datalake/datalake_path_client.hpp"
#include "datalake/datalake_utilities.hpp"
#include "http/curl/curl.hpp"

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
          fileSystemUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return FileSystemClient(fileSystemUri.ToString(), options);
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
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
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
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
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

  Azure::Core::Response<FileSystemInfo> FileSystemClient::Create(
      const FileSystemCreateOptions& options) const
  {
    Blobs::CreateContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Metadata = options.Metadata;
    auto result = m_blobContainerClient.Create(blobOptions);
    FileSystemInfo ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<FileSystemInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileSystemDeleteResponse> FileSystemClient::Delete(
      const FileSystemDeleteOptions& options) const
  {
    Blobs::DeleteContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions);
    FileSystemDeleteResponse ret;
    return Azure::Core::Response<FileSystemDeleteResponse>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileSystemProperties> FileSystemClient::GetProperties(
      const FileSystemGetPropertiesOptions& options) const
  {
    Blobs::GetContainerPropertiesOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions);
    FileSystemProperties ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Metadata = std::move(result->Metadata);
    return Azure::Core::Response<FileSystemProperties>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileSystemInfo> FileSystemClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const FileSystemSetMetadataOptions& options) const
  {
    Blobs::SetContainerMetadataOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    auto result = m_blobContainerClient.SetMetadata(metadata, blobOptions);
    FileSystemInfo ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    return Azure::Core::Response<FileSystemInfo>(std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<FileSystemListPathsResponse> FileSystemClient::ListPaths(
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
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

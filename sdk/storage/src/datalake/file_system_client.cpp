// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/file_system_client.hpp"

#include "blobs/internal/protocol/blob_rest_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake//directory_client.hpp"
#include "datalake//file_client.hpp"
#include "datalake/datalake_utilities.hpp"
#include "datalake/path_client.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobContainerClientOptions GetBlobContainerClientOptions(
        const FileSystemClientOptions& options)
    {
      Blobs::BlobContainerClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(
            std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(
            std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
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
      : m_blobContainerClient(
          Details::GetBlobUriFromUri(fileSystemUri),
          credential,
          GetBlobContainerClientOptions(options)),
        m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri))
  {

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<TokenCredential> credential,
      const FileSystemClientOptions& options)
      : m_blobContainerClient(
          Details::GetBlobUriFromUri(fileSystemUri),
          credential,
          GetBlobContainerClientOptions(options)),
        m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<TokenCredentialPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      const FileSystemClientOptions& options)
      : m_blobContainerClient(
          Details::GetBlobUriFromUri(fileSystemUri),
          GetBlobContainerClientOptions(options)),
        m_dfsUri(Details::GetDfsUriFromUri(fileSystemUri))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
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

  FileSystemCreateResponse FileSystemClient::Create(const FileSystemCreateOptions& options) const
  {
    DataLakeRestClient::FileSystem::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Properties = Details::SerializeMetadata(options.Metadata);
    return DataLakeRestClient::FileSystem::Create(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemDeleteResponse FileSystemClient::Delete(const FileSystemDeleteOptions& options) const
  {
    DataLakeRestClient::FileSystem::DeleteOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return DataLakeRestClient::FileSystem::Delete(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemProperties FileSystemClient::GetProperties(
      const FileSystemGetPropertiesOptions& options) const
  {
    DataLakeRestClient::FileSystem::GetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    auto result = DataLakeRestClient::FileSystem::GetProperties(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
    auto ret = FileSystemProperties();
    ret.Date = std::move(result.Date);
    ret.ETag = std::move(result.ETag);
    ret.LastModified = std::move(result.LastModified);
    ret.RequestId = std::move(result.RequestId);
    ret.Version = std::move(result.Version);
    ret.Metadata = Details::DeserializeMetadata(result.Properties);
    ret.NamespaceEnabled = result.NamespaceEnabled == "true" ? true : false;
    return ret;
  }

  FileSystemSetPropertiesResponse FileSystemClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const FileSystemSetMetadataOptions& options) const
  {
    DataLakeRestClient::FileSystem::SetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Properties = Details::SerializeMetadata(metadata);
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return DataLakeRestClient::FileSystem::SetProperties(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemListPathsResponse FileSystemClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options) const
  {
    DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Directory = options.Directory;
    protocolLayerOptions.RecursiveRequired = recursive;
    return DataLakeRestClient::FileSystem::ListPaths(
        m_dfsUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake

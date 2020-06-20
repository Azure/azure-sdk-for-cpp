// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/file_system_client.hpp"

#include "blobs/internal/protocol/blob_rest_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/constant.hpp"
#include "common/crypt.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake/datalake_utilities.hpp"
#include "datalake/path_client.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace DataLake {

  FileSystemClient FileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const FileSystemClientOptions& options)
  {
    auto parsedConnectionString = ParseConnectionString(connectionString);

    std::string accountName;
    std::string accountKey;
    std::string blobEndpoint;
    std::string datalakeEndpoint;
    std::string EndpointSuffix;
    std::string defaultEndpointsProtocol = Details::c_PathDnsSuffixDefault;

    auto ite
        = parsedConnectionString.find(Azure::Storage::Details::c_ConnectionStringTagAccountName);
    if (ite != parsedConnectionString.end())
    {
      accountName = ite->second;
    }
    ite = parsedConnectionString.find(Azure::Storage::Details::c_ConnectionStringTagAccountKey);
    if (ite != parsedConnectionString.end())
    {
      accountKey = ite->second;
    }
    ite = parsedConnectionString.find(
        Azure::Storage::Details::c_ConnectionStringTagDataLakeEndpoint);
    if (ite != parsedConnectionString.end())
    {
      datalakeEndpoint = ite->second;
    }
    else
    {
      // Blob endpoint should also work due to interop. But honor DFS endpoint first.
      ite = parsedConnectionString.find(Azure::Storage::Details::c_ConnectionStringTagBlobEndpoint);
      if (ite != parsedConnectionString.end())
      {
        blobEndpoint
            = ("." + (Azure::Storage::Details::c_DfsEndpointIdentifier + ("." + ite->second)));
      }
    }
    ite = parsedConnectionString.find(Azure::Storage::Details::c_ConnectionStringTagEndpointSuffix);
    if (ite != parsedConnectionString.end())
    {
      EndpointSuffix = ite->second;
    }
    ite = parsedConnectionString.find(
        Azure::Storage::Details::c_ConnectionStringTagDefaultEndpointsProtocol);
    if (ite != parsedConnectionString.end())
    {
      defaultEndpointsProtocol = ite->second;
    }

    UrlBuilder builder;
    builder.SetScheme(defaultEndpointsProtocol);
    if (!datalakeEndpoint.empty())
    {
      builder = UrlBuilder(datalakeEndpoint);
    }
    else if (!blobEndpoint.empty())
    {
      builder = UrlBuilder(blobEndpoint);
    }
    else if (!accountName.empty())
    {
      builder.SetHost(accountName + ".dfs." + EndpointSuffix);
    }
    else
    {
      throw std::runtime_error("invalid connection string");
    }

    builder.AppendPath(fileSystemName, true);
    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return FileSystemClient(builder.to_string(), credential, options);
  }

  FileSystemClient::FileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const FileSystemClientOptions& options)
      : m_dfsUri(fileSystemUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
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
      : m_dfsUri(fileSystemUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
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
      : m_dfsUri(fileSystemUri)
  {
    m_blobUri = Details::GetBlobUriFromDfsUri(m_dfsUri);

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
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
    PathClient client = PathClient();
    auto builder = m_dfsUri;
    builder.AppendPath(path, true);
    client.m_dfsUri = std::move(builder);
    client.m_blobUri = Details::GetBlobUriFromDfsUri(builder);
    client.m_pipeline = m_pipeline;
    return client;
  }

  FileSystemCreateResponse FileSystemClient::Create(const FileSystemCreateOptions& options) const
  {
    DataLakeRestClient::FileSystem::CreateOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Properties = SerializeMetadata(options.Metadata);
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::FileSystem::Create(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemDeleteResponse FileSystemClient::Delete(const FileSystemDeleteOptions& options) const
  {
    DataLakeRestClient::FileSystem::DeleteOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::FileSystem::Delete(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemGetMetadataResponse FileSystemClient::GetMetadata(
      const FileSystemGetMetadataOptions& options) const
  {
    DataLakeRestClient::FileSystem::GetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Timeout = options.Timeout;
    auto result = DataLakeRestClient::FileSystem::GetProperties(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
    return FileSystemGetMetadataResponse{
        std::move(result.Date),
        std::move(result.ETag),
        std::move(result.LastModified),
        std::move(result.RequestId),
        std::move(result.Version),
        DeserializeMetadata(result.Properties),
        result.NamespaceEnabled == "true" ? true : false};
  }

  FileSystemSetPropertiesResponse FileSystemClient::SetMetadata(
      const std::map<std::string, std::string>& metadata,
      const FileSystemSetMetadataOptions& options) const
  {
    DataLakeRestClient::FileSystem::SetPropertiesOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Properties = SerializeMetadata(metadata);
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::FileSystem::SetProperties(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
  }

  FileSystemListPathsResponse FileSystemClient::ListPaths(
      bool recursive,
      const ListPathsOptions& options) const
  {
    DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Upn = options.Upn;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Directory = options.Directory;
    protocolLayerOptions.RecursiveRequired = recursive;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::FileSystem::ListPaths(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::DataLake

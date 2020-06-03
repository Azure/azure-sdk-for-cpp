// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/service_client.hpp"

#include "blobs/internal/protocol/blob_rest_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/constant.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/token_credential_policy.hpp"
#include "datalake/datalake_utilities.hpp"
#include "datalake/file_system_client.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace DataLake {

  ServiceClient ServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ServiceClientOptions& options)
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

    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return ServiceClient(builder.to_string(), credential, options);
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(serviceUri)
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

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<TokenCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(serviceUri)
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

  ServiceClient::ServiceClient(const std::string& serviceUri, const ServiceClientOptions& options)
      : m_dfsUri(serviceUri)
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

  FileSystemClient ServiceClient::GetFileSystemClient(const std::string& fileSystemName) const
  {
    FileSystemClient client = FileSystemClient();
    auto builder = m_dfsUri;
    builder.AppendPath(fileSystemName, true);
    client.m_dfsUri = std::move(builder);
    client.m_blobUri = Details::GetBlobUriFromDfsUri(builder);
    client.m_pipeline = m_pipeline;
    return client;
  }

  ServiceListFileSystemsResponse ServiceClient::ListFileSystems(
      const ListFileSystemsOptions& options) const
  {
    DataLakeRestClient::Service::ListFileSystemsOptions protocolLayerOptions;
    // TODO: Add null check here when Nullable<T> is supported
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Continuation = options.Continuation;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Timeout = options.Timeout;
    return DataLakeRestClient::Service::ListFileSystems(
        m_dfsUri.to_string(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::DataLake

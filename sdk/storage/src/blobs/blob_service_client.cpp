// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_service_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobServiceClient BlobServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobServiceClientOptions& options)
  {
    auto parsedConnectionString = ParseConnectionString(connectionString);

    std::string accountName;
    std::string accountKey;
    std::string blobEndpoint;
    std::string EndpointSuffix;
    std::string defaultEndpointsProtocol = ".core.windows.net";

    auto ite = parsedConnectionString.find("AccountName");
    if (ite != parsedConnectionString.end())
    {
      accountName = ite->second;
    }
    ite = parsedConnectionString.find("AccountKey");
    if (ite != parsedConnectionString.end())
    {
      accountKey = ite->second;
    }
    ite = parsedConnectionString.find("BlobEndpoint");
    if (ite != parsedConnectionString.end())
    {
      blobEndpoint = ite->second;
    }
    ite = parsedConnectionString.find("EndpointSuffix");
    if (ite != parsedConnectionString.end())
    {
      EndpointSuffix = ite->second;
    }
    ite = parsedConnectionString.find("DefaultEndpointsProtocol");
    if (ite != parsedConnectionString.end())
    {
      defaultEndpointsProtocol = ite->second;
    }

    UrlBuilder builder;
    builder.SetScheme(defaultEndpointsProtocol);
    if (!blobEndpoint.empty())
    {
      builder = UrlBuilder(blobEndpoint);
    }
    else if (!accountName.empty())
    {
      builder.SetHost(accountName + ".blob." + EndpointSuffix);
    }
    else
    {
      throw std::runtime_error("invalid connection string");
    }

    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return BlobServiceClient(builder.to_string(), credential, options);
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobServiceClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
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

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<TokenCredential> credential,
      const BlobServiceClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    // not implemented yet
    unused(credential);
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUri,
      const BlobServiceClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
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

  ListContainersSegment BlobServiceClient::ListBlobContainersSegment(
      const ListBlobContainersOptions& options) const
  {
    BlobRestClient::Service::ListBlobContainersOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.IncludeMetadata = ListBlobContainersIncludeOption::None;
    for (auto i : options.Include)
    {
      if (i == ListBlobContainersIncludeOption::Metadata)
      {
        protocolLayerOptions.IncludeMetadata = i;
      }
    }
    return BlobRestClient::Service::ListBlobContainers(
        options.Context, *m_pipeline, m_serviceUrl.to_string(), protocolLayerOptions);
  }

  UserDelegationKey BlobServiceClient::GetUserDelegationKey(
      const std::string& expiresOn,
      const GetUserDelegationKeyOptions& options) const
  {
    BlobRestClient::Service::GetUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.StartsOn = options.StartsOn;
    protocolLayerOptions.ExpiresOn = expiresOn;
    return BlobRestClient::Service::GetUserDelegationKey(
        options.Context, *m_pipeline, m_serviceUrl.to_string(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

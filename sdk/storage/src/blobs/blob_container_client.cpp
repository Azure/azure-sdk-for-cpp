// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_container_client.hpp"

#include "common/storage_common.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobContainerClient BlobContainerClient::FromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const BlobContainerClientOptions& options)
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
      builder.SetHost(accountName + ".blob" + defaultEndpointsProtocol);
    }
    else
    {
      throw std::runtime_error("invalid connection string");
    }
    builder.AppendPath(containerName, true);

    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return BlobContainerClient(builder.to_string(), credential, options);
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobContainerClientOptions& options)
      : BlobContainerClient(containerUri, options)
  {
    // not implemented yet
    unused(credential);
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      std::shared_ptr<TokenCredential> credential,
      const BlobContainerClientOptions& options)
      : BlobContainerClient(containerUri, options)
  {
    // not implemented yet
    unused(credential);
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      const BlobContainerClientOptions& options)
      : m_ContainerUri(containerUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobContainerInfo BlobContainerClient::Create(const CreateBlobContainerOptions& options)
  {
    BlobRestClient::Container::CreateOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    return BlobRestClient::Container::Create(
        options.context, *m_pipeline, m_ContainerUri.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobContainerClient::Delete(const DeleteBlobContainerOptions& options)
  {
    unused(options);
    BlobRestClient::Container::DeleteOptions protocolLayerOptions;
    return BlobRestClient::Container::Delete(
        options.context, *m_pipeline, m_ContainerUri.to_string(), protocolLayerOptions);
  }

  BlobContainerProperties BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options)
  {
    unused(options);
    BlobRestClient::Container::GetPropertiesOptions protocolLayerOptions;
    return BlobRestClient::Container::GetProperties(
        options.context, *m_pipeline, m_ContainerUri.to_string(), protocolLayerOptions);
  }

  BlobContainerInfo BlobContainerClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      SetBlobContainerMetadataOptions options)
  {
    unused(options);
    BlobRestClient::Container::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    return BlobRestClient::Container::SetMetadata(
        options.context, *m_pipeline, m_ContainerUri.to_string(), protocolLayerOptions);
  }

  BlobsFlatSegment BlobContainerClient::ListBlobs(const ListBlobsOptions& options)
  {
    BlobRestClient::Container::ListBlobsOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = options.Delimiter;
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    return BlobRestClient::Container::ListBlobs(
        options.context, *m_pipeline, m_ContainerUri.to_string(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

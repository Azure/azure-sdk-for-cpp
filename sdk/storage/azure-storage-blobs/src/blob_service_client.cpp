// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_service_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobServiceClient BlobServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobServiceClientOptions& options)
  {
    auto parsedConnectionString = Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.BlobServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobServiceClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobServiceClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobServiceClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
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

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const BlobServiceClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Details::c_StorageScope));
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
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobContainerClient BlobServiceClient::GetBlobContainerClient(
      const std::string& containerName) const
  {
    auto containerUri = m_serviceUrl;
    containerUri.AppendPath(Details::UrlEncodePath(containerName));
    return BlobContainerClient(std::move(containerUri), m_pipeline);
  }

  Azure::Core::Response<ListContainersSegmentResult> BlobServiceClient::ListBlobContainersSegment(
      const ListContainersSegmentOptions& options) const
  {
    BlobRestClient::Service::ListContainersSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    return BlobRestClient::Service::ListBlobContainers(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetUserDelegationKeyResult> BlobServiceClient::GetUserDelegationKey(
      const std::string& startsOn,
      const std::string& expiresOn,
      const GetUserDelegationKeyOptions& options) const
  {
    BlobRestClient::Service::GetUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.StartsOn = startsOn;
    protocolLayerOptions.ExpiresOn = expiresOn;
    return BlobRestClient::Service::GetUserDelegationKey(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<SetServicePropertiesResult> BlobServiceClient::SetProperties(
      BlobServiceProperties properties,
      const SetServicePropertiesOptions& options) const
  {
    BlobRestClient::Service::SetServicePropertiesOptions protocolLayerOptions;
    protocolLayerOptions.Properties = std::move(properties);
    return BlobRestClient::Service::SetProperties(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetServicePropertiesResult> BlobServiceClient::GetProperties(
      const GetServicePropertiesOptions& options) const
  {
    BlobRestClient::Service::GetServicePropertiesOptions protocolLayerOptions;
    return BlobRestClient::Service::GetProperties(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetAccountInfoResult> BlobServiceClient::GetAccountInfo(
      const GetAccountInfoOptions& options) const
  {
    BlobRestClient::Service::GetAccountInfoOptions protocolLayerOptions;
    return BlobRestClient::Service::GetAccountInfo(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetServiceStatisticsResult> BlobServiceClient::GetStatistics(
      const GetBlobServiceStatisticsOptions& options) const
  {
    BlobRestClient::Service::GetServiceStatisticsOptions protocolLayerOptions;
    return BlobRestClient::Service::GetStatistics(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<FindBlobsByTagsResult> BlobServiceClient::FindBlobsByTags(
      const std::string& tagFilterSqlExpression,
      const FindBlobsByTagsOptions& options) const
  {
    BlobRestClient::Service::FilterBlobsSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Where = tagFilterSqlExpression;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    return BlobRestClient::Service::FilterBlobs(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

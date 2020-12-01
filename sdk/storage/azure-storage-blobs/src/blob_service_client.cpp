// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_service_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobServiceClient BlobServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::Details::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.BlobServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobServiceClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobServiceClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
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
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
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
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Storage::Details::StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
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
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobContainerClient BlobServiceClient::GetBlobContainerClient(
      const std::string& blobContainerName) const
  {
    auto blobContainerUrl = m_serviceUrl;
    blobContainerUrl.AppendPath(Storage::Details::UrlEncodePath(blobContainerName));
    return BlobContainerClient(
        std::move(blobContainerUrl), m_pipeline, m_customerProvidedKey, m_encryptionScope);
  }

  Azure::Core::Response<Models::ListBlobContainersSegmentResult>
  BlobServiceClient::ListBlobContainersSegment(
      const ListBlobContainersSegmentOptions& options) const
  {
    Details::BlobRestClient::Service::ListBlobContainersSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    return Details::BlobRestClient::Service::ListBlobContainers(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetUserDelegationKeyResult> BlobServiceClient::GetUserDelegationKey(
      const std::string& startsOn,
      const std::string& expiresOn,
      const GetUserDelegationKeyOptions& options) const
  {
    Details::BlobRestClient::Service::GetUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.StartsOn = startsOn;
    protocolLayerOptions.ExpiresOn = expiresOn;
    return Details::BlobRestClient::Service::GetUserDelegationKey(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetServicePropertiesResult> BlobServiceClient::SetProperties(
      Models::BlobServiceProperties properties,
      const SetServicePropertiesOptions& options) const
  {
    Details::BlobRestClient::Service::SetServicePropertiesOptions protocolLayerOptions;
    protocolLayerOptions.Properties = std::move(properties);
    return Details::BlobRestClient::Service::SetProperties(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetServicePropertiesResult> BlobServiceClient::GetProperties(
      const GetServicePropertiesOptions& options) const
  {
    Details::BlobRestClient::Service::GetServicePropertiesOptions protocolLayerOptions;
    return Details::BlobRestClient::Service::GetProperties(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetAccountInfoResult> BlobServiceClient::GetAccountInfo(
      const GetAccountInfoOptions& options) const
  {
    Details::BlobRestClient::Service::GetAccountInfoOptions protocolLayerOptions;
    return Details::BlobRestClient::Service::GetAccountInfo(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetServiceStatisticsResult> BlobServiceClient::GetStatistics(
      const GetBlobServiceStatisticsOptions& options) const
  {
    Details::BlobRestClient::Service::GetServiceStatisticsOptions protocolLayerOptions;
    return Details::BlobRestClient::Service::GetStatistics(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::FindBlobsByTagsResult> BlobServiceClient::FindBlobsByTags(
      const std::string& tagFilterSqlExpression,
      const FindBlobsByTagsOptions& options) const
  {
    Details::BlobRestClient::Service::FilterBlobsSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Where = tagFilterSqlExpression;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    return Details::BlobRestClient::Service::FilterBlobs(
        options.Context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs

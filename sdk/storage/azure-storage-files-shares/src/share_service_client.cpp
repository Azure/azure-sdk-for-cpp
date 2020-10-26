// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_service_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  ServiceClient ServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ServiceClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.FileServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return ServiceClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ServiceClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ServiceClientOptions& options)
      : m_serviceUri(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
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

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const ServiceClientOptions& options)
      : m_serviceUri(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
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
        credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ServiceClient::ServiceClient(const std::string& serviceUri, const ServiceClientOptions& options)
      : m_serviceUri(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient ServiceClient::GetShareClient(const std::string& shareName) const
  {
    auto builder = m_serviceUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(shareName));
    return ShareClient(builder, m_pipeline);
  }

  Azure::Core::Response<ListSharesSegmentResult> ServiceClient::ListSharesSegment(
      const ListSharesSegmentOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Service::ListSharesSegmentOptions();
    protocolLayerOptions.ListSharesInclude = options.ListSharesInclude;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Prefix = options.Prefix;
    return ShareRestClient::Service::ListSharesSegment(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<SetServicePropertiesResult> ServiceClient::SetProperties(
      StorageServiceProperties properties,
      const SetServicePropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Service::SetPropertiesOptions();
    protocolLayerOptions.ServiceProperties = std::move(properties);
    return ShareRestClient::Service::SetProperties(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<GetServicePropertiesResult> ServiceClient::GetProperties(
      const GetServicePropertiesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Service::GetPropertiesOptions();
    auto result = ShareRestClient::Service::GetProperties(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
    StorageServiceProperties ret;
    ret.Cors = std::move(result->Cors);
    ret.HourMetrics = std::move(result->HourMetrics);
    ret.MinuteMetrics = std::move(result->MinuteMetrics);
    ret.Protocol = std::move(result->Protocol);
    return Azure::Core::Response<StorageServiceProperties>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::Shares

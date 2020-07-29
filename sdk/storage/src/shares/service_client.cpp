// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "shares/service_client.hpp"

#include "common/common_headers_request_policy.hpp"
#include "common/constants.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_version.hpp"
#include "credentials/policy/policies.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  ServiceClient ServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ServiceClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.DataLakeServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return ServiceClient(serviceUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ServiceClient(serviceUri.ToString(), options);
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
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
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

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const ServiceClientOptions& options)
      : m_serviceUri(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
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

  ServiceClient::ServiceClient(const std::string& serviceUri, const ServiceClientOptions& options)
      : m_serviceUri(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_FileServicePackageName, FileServiceVersion));
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

  Azure::Core::Response<ListSharesSegmentResult> ServiceClient::ListSharesSegment(
      const ListSharesOptions& options) const
  {
    auto protocolLayerOptions = ShareRestClient::Service::ListSharesSegmentOptions();
    protocolLayerOptions.ListSharesInclude = std::move(options.ListSharesInclude);
    protocolLayerOptions.Marker = std::move(options.Marker);
    protocolLayerOptions.MaxResults = std::move(options.MaxResults);
    protocolLayerOptions.Prefix = std::move(options.Prefix);
    return ShareRestClient::Service::ListSharesSegment(
        m_serviceUri.ToString(), *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::Shares

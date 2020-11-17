// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_service_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  ShareServiceClient ShareServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ShareServiceClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.FileServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return ShareServiceClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ShareServiceClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ShareServiceClientOptions& options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const ShareServiceClientOptions& options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareServiceClient::ShareServiceClient(
      const std::string& serviceUri,
      const ShareServiceClientOptions& options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ShareClient ShareServiceClient::GetShareClient(const std::string& shareName) const
  {
    auto builder = m_serviceUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(shareName));
    return ShareClient(builder, m_pipeline);
  }

  Azure::Core::Response<Models::ListSharesSegmentResult> ShareServiceClient::ListSharesSegment(
      const ListSharesSegmentOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Service::ListSharesSegmentOptions();
    protocolLayerOptions.ListSharesInclude = options.ListSharesInclude;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Prefix = options.Prefix;
    return Details::ShareRestClient::Service::ListSharesSegment(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetServicePropertiesResult> ShareServiceClient::SetProperties(
      Models::StorageServiceProperties properties,
      const SetServicePropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Service::SetPropertiesOptions();
    protocolLayerOptions.ServiceProperties = std::move(properties);
    return Details::ShareRestClient::Service::SetProperties(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetServicePropertiesResult> ShareServiceClient::GetProperties(
      const GetServicePropertiesOptions& options) const
  {
    auto protocolLayerOptions = Details::ShareRestClient::Service::GetPropertiesOptions();
    auto result = Details::ShareRestClient::Service::GetProperties(
        m_serviceUri, *m_pipeline, options.Context, protocolLayerOptions);
    Models::StorageServiceProperties ret;
    ret.Cors = std::move(result->Cors);
    ret.HourMetrics = std::move(result->HourMetrics);
    ret.MinuteMetrics = std::move(result->MinuteMetrics);
    ret.Protocol = std::move(result->Protocol);
    return Azure::Core::Response<Models::StorageServiceProperties>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::Shares
